#include "georoute/http_server.hpp"

#include <optional>
#include <utility>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "georoute/engine.hpp"

namespace georoute {

namespace {

nlohmann::json make_health_response() {
    return nlohmann::json{{"status", "ok"}};
}

nlohmann::json make_error_response(const std::string& message) {
    return nlohmann::json{{"error", message}};
}

template <typename Handler>
void wrap_endpoint(httplib::Server& server, const std::string& path, Handler&& handler) {
    server.Post(path.c_str(), [handler = std::forward<Handler>(handler)](const httplib::Request& req,
                                                                         httplib::Response& res) {
        try {
            handler(req, res);
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(make_error_response(ex.what()).dump(), "application/json");
        } catch (...) {
            res.status = 500;
            res.set_content(make_error_response("internal server error").dump(), "application/json");
        }
    });
}

std::optional<nlohmann::json> parse_json(const httplib::Request& req) {
    nlohmann::json body = nlohmann::json::parse(req.body, nullptr, false);
    if (body.is_discarded()) {
        return std::nullopt;
    }
    return body;
}

}  // namespace

int run_http_server(GeoRouteEngine& engine, const HttpServerOptions& options) {
    httplib::Server server;

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        const auto payload = make_health_response();
        res.set_content(payload.dump(), "application/json");
    });

    server.Get("/api/v1/health", [](const httplib::Request&, httplib::Response& res) {
        const auto payload = make_health_response();
        res.set_content(payload.dump(), "application/json");
    });

    server.Get("/route", [&engine](const httplib::Request& req, httplib::Response& res) {
        const auto src_param = req.get_param_value("src");
        const auto dst_param = req.get_param_value("dst");
        
        if (src_param.empty() || dst_param.empty()) {
            res.status = 400;
            res.set_content(make_error_response("missing 'src' or 'dst' query parameters").dump(), "application/json");
            return;
        }
        
        try {
            const auto source = static_cast<node_id>(std::stoul(src_param));
            const auto target = static_cast<node_id>(std::stoul(dst_param));
            
            const auto response = engine.route(source, target);
            
            nlohmann::json json_response{
                {"src", source},
                {"dst", target},
                {"distance", response.result.total_travel_time},
                {"eta_ms", static_cast<int>(response.result.total_travel_time * 1000)},
                {"path", response.result.nodes},
                {"reachable", response.result.reachable},
                {"stats", {
                    {"compute_us", response.compute_time_us},
                    {"expanded_nodes", response.expanded_nodes}
                }}
            };
            
            res.set_content(json_response.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(make_error_response(ex.what()).dump(), "application/json");
        }
    });

    wrap_endpoint(server, "/api/v1/route", [&engine](const httplib::Request& req, httplib::Response& res) {
        const auto payload = parse_json(req);
        if (!payload) {
            res.status = 400;
            res.set_content(make_error_response("invalid JSON payload").dump(), "application/json");
            return;
        }
        if (!payload->contains("source") || !payload->contains("target")) {
            res.status = 400;
            res.set_content(make_error_response("missing 'source' or 'target'").dump(), "application/json");
            return;
        }

        const auto source = payload->at("source").get<node_id>();
        const auto target = payload->at("target").get<node_id>();

        const auto response = engine.route(source, target);
        nlohmann::json json_response{
            {"src", source},
            {"dst", target},
            {"distance", response.result.total_travel_time},
            {"eta_ms", static_cast<int>(response.result.total_travel_time * 1000)},
            {"path", response.result.nodes},
            {"reachable", response.result.reachable},
            {"stats", {
                {"compute_us", response.compute_time_us},
                {"expanded_nodes", response.expanded_nodes}
            }}
        };

        res.set_content(json_response.dump(), "application/json");
    });

    wrap_endpoint(server, "/api/v1/congestion/update", [&engine](const httplib::Request& req, httplib::Response& res) {
        const auto payload = parse_json(req);
        if (!payload) {
            res.status = 400;
            res.set_content(make_error_response("invalid JSON payload").dump(), "application/json");
            return;
        }

        if (!payload->contains("edge_start") || !payload->contains("edge_end") || !payload->contains("factor")) {
            res.status = 400;
            res.set_content(make_error_response("missing 'edge_start', 'edge_end', or 'factor'").dump(),
                            "application/json");
            return;
        }

        const auto edge_start = payload->at("edge_start").get<std::size_t>();
        const auto edge_end = payload->at("edge_end").get<std::size_t>();
        const auto factor = payload->at("factor").get<float>();
        engine.apply_congestion_update(edge_start, edge_end, factor);

        res.set_content(nlohmann::json{{"status", "ok"}}.dump(), "application/json");
    });

    server.Get("/metrics", [&engine](const httplib::Request&, httplib::Response& res) {
        const auto stats = engine.get_stats();
        nlohmann::json metrics{
            {"queries_total", stats.total_queries},
            {"updates_total", stats.total_updates},
            {"compute_time_total_us", stats.total_compute_time_us},
            {"compute_time_max_us", stats.max_compute_time_us},
            {"compute_time_avg_us", stats.total_queries > 0 ? stats.total_compute_time_us / stats.total_queries : 0.0}
        };
        res.set_content(metrics.dump(2), "application/json");
    });

    const auto success = server.listen(options.host.c_str(), static_cast<int>(options.port));
    return success ? 0 : 1;
}

}  // namespace georoute

