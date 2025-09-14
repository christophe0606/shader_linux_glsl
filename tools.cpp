#include "config.h"
extern "C" {
#include "mcp.h"
}

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vec3 edgeColor;
vec3 tileAColor;
vec3 tileBColor;
float thickness;
int geometryType;
int symmetry;
int animationOn;



int setColor(const char* name,vec3*  color)
{
    color->x=1.0  ; color->y=1.0; color->z=1.0;

    if (strcmp(name,"red")==0)
    {
        color->x=1.0  ; color->y=0.0;color->z=0.0;
        return 0;
    }

    if (strcmp(name,"green")==0)
    {
        color->x=0.0  ; color->y=1.0;color->z=0.0;
        return 0;
    }

    if (strcmp(name,"blue")==0)
    {
        color->x=0.0  ; color->y=0.0;color->z=1.0;
        return 0;
    }

    if (strcmp(name,"white")==0)
    {
        color->x=1.0  ; color->y=1.0;color->z=1.0;
        return 0;
    }

    if (strcmp(name,"black")==0)
    {
        color->x=0.0  ; color->y=0.0;color->z=0.0;
        return 0;
    }

    if (strcmp(name,"gray")==0)
    {
        color->x=0.5  ; color->y=0.5;color->z=0.5;
        return 0;
    }

    return 1;
}

void setDefaults()
{
    (void)setColor("black",&edgeColor);
    (void)setColor("red",&tileAColor);
    (void)setColor("blue",&tileBColor);
    thickness=0.01;
    geometryType=0;
    symmetry=0;
    animationOn=1;
}

static cJSON *tool_set_color_of_setting(cJSON *args)
{
    const cJSON *object = cJSON_GetObjectItemCaseSensitive(args, "object");
    const cJSON *color = cJSON_GetObjectItemCaseSensitive(args, "color");
    const char *col = (cJSON_IsString(color) && color->valuestring) ? color->valuestring : "";
    const char *obj = (cJSON_IsString(object) && object->valuestring) ? object->valuestring : "";

    int err = setColor(col,&edgeColor);
    cJSON *res;
    if (err)
    {
      res=create_result_text("unknown color");
    }
    else
    {
      res=create_result_text("color changed");
    }
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
        return err(id, MCP_INVALID_PARAMS, "Invalid params");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(params, "name");
    const cJSON *arguments = cJSON_GetObjectItemCaseSensitive(params, "arguments");
    if (!cJSON_IsString(name) || !name->valuestring)
        return err(id, MCP_METHOD_NOT_FOUND, "Missing tool name");
    if (!cJSON_IsObject(arguments))
        return err(id, MCP_INVALID_PARAMS, "Missing arguments");

    cJSON *result = NULL;
    if (strcmp(name->valuestring, "colorSetting") == 0)
    {
        result = tool_set_color_of_setting((cJSON *)arguments);
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

    setDefaults();

    struct tool *colorTool = add_tool("colorSetting", "Set color of object");
    add_argument(colorTool, 
        "color", TYPE_STR, "Color name (red, green, blue, white,black,gray)");
    add_argument(colorTool, 
        "object", TYPE_STR, "Object (edge,background,first tile,second tile)");

    struct tool *addTool = add_tool("add", "Add two numbers");
    // Add arguments in reverse order (linked list)
    add_argument(addTool, "b", TYPE_FLOAT, "Second number");
    add_argument(addTool, "a", TYPE_FLOAT, "First number");
}