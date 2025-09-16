# Used to debug the MCP protocol on HTTP
mitmdump --mode reverse:http://127.0.0.1:8100/ -p 8080 -s dump.py
