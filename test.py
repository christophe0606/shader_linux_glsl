from fastmcp import FastMCP
import fastmcp.utilities.logging

#logger = fastmcp.utilities.logging.get_logger("SERVER")
fastmcp.utilities.logging.configure_logging("DEBUG")
mcp = FastMCP("Demo 🚀")

@mcp.tool
def add(a: int, b: int) -> int:
    """Add two numbers"""
    return a + b

if __name__ == "__main__":
    mcp.run(transport="http", host="127.0.0.1", port=8100)