#include "config.h"
extern "C" {
#include "mcp.h"
}

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vec3 edgeColor;

void setColor(const char* name,vec3*  color)
{
    color->x=1.0  ; color->y=1.0; color->z=1.0;

    if (strcmp(name,"red")==0)
    {
        color->x=1.0  ; color->y=0.0;color->z=0.0;
    }

    if (strcmp(name,"green")==0)
    {
        color->x=0.0  ; color->y=1.0;color->z=0.0;
    }

    if (strcmp(name,"blue")==0)
    {
        color->x=0.0  ; color->y=0.0;color->z=1.0;
    }

    if (strcmp(name,"white")==0)
    {
        color->x=1.0  ; color->y=1.0;color->z=1.0;
    }

    if (strcmp(name,"black")==0)
    {
        color->x=0.0  ; color->y=0.0;color->z=0.0;
    }

    if (strcmp(name,"gray")==0)
    {
        color->x=0.5  ; color->y=0.5;color->z=0.5;
    }
}

static cJSON *tool_set_edge_color(cJSON *args)
{
    const cJSON *text = cJSON_GetObjectItemCaseSensitive(args, "color");
    const char *s = (cJSON_IsString(text) && text->valuestring) ? text->valuestring : "";
    setColor(s,&edgeColor);
    cJSON *res = create_result_text("ok");
    return res;
}

static cJSON *tool_add(cJSON *args)
{
    const cJSON *a = cJSON_GetObjectItemCaseSensitive(args, "a");
    const cJSON *b = cJSON_GetObjectItemCaseSensitive(args, "b");
    double ad = cJSON_IsNumber(a) ? a->valuedouble : 0.0;
    double bd = cJSON_IsNumber(b) ? b->valuedouble : 0.0;
    double sum = ad + bd;

    char buf[64];
    snprintf(buf, sizeof(buf), "%.17g", sum);

    cJSON *res = create_result_text(buf);

    return res;
}

cJSON *handle_tools_call(cJSON *id, cJSON *params)
{
    if (!cJSON_IsObject(params))
        return err(id, -32602, "Invalid params");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(params, "name");
    const cJSON *arguments = cJSON_GetObjectItemCaseSensitive(params, "arguments");
    if (!cJSON_IsString(name) || !name->valuestring)
        return err(id, -32602, "Missing tool name");
    if (!cJSON_IsObject(arguments))
        return err(id, -32602, "Missing arguments");

    cJSON *result = NULL;
    if (strcmp(name->valuestring, "edgeColor") == 0)
    {
        result = tool_set_edge_color((cJSON *)arguments);
    }
    else if (strcmp(name->valuestring, "add") == 0)
    {
        result = tool_add((cJSON *)arguments);
    }
    else
    {
        return err(id, -32601, "Unknown tool");
    }
    return ok(id, result);
}

void define_tools()
{

    setColor("red",&edgeColor);

    struct tool *edgeColorTool = add_tool("edgeColor", "Set color of edges");
    add_argument(edgeColorTool, "color", TYPE_STR, "Color name (red, green, blue, white,black,gray)");

    struct tool *addTool = add_tool("add", "Add two numbers");
    // Add arguments in reverse order (linked list)
    add_argument(addTool, "b", TYPE_FLOAT, "Second number");
    add_argument(addTool, "a", TYPE_FLOAT, "First number");
}