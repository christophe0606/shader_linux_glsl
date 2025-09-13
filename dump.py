from mitmproxy import http

def response(flow: http.HTTPFlow) -> None:
    # Log request
    with open("requests.log", "a", encoding="utf-8") as f:
        f.write("=== REQUEST ===\n")
        f.write(f"{flow.request.method} {flow.request.pretty_url}\n")
        for name, value in flow.request.headers.items():
            f.write(f"{name}: {value}\n")
        f.write("\n")
        if flow.request.content:
            f.write(flow.request.get_text() + "\n")
        f.write("\n")

    # Log response
    with open("responses.log", "a", encoding="utf-8") as f:
        f.write("=== RESPONSE ===\n")
        f.write(f"HTTP/{flow.response.http_version} {flow.response.status_code}\n")
        for name, value in flow.response.headers.items():
            f.write(f"{name}: {value}\n")
        f.write("\n")
        if flow.response.content:
            f.write(flow.response.get_text() + "\n")
        f.write("\n")