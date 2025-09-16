/*

Definition of the MCP tools exposed by the application.

*/
#include "config.h"
extern "C" {
#include "mcp.h"
}

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vec3 edgeColor;
vec3 backgroundColor;
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
    (void)setColor("black",&backgroundColor);
    thickness=0.01;
    geometryType=0;
    symmetry=0;
    animationOn=1;
}

static cJSON *tool_reset(cJSON *args)
{
    setDefaults();
    return create_result_text("reset to defaults");
}


static cJSON *tool_set_animation(cJSON *args)
{
    const cJSON *status = cJSON_GetObjectItemCaseSensitive(args, "on");
    const int error = cJSON_IsBool(status) ? 0 : 1;
    const cJSON_bool animaStatus = cJSON_IsBool(status) && cJSON_IsTrue(status);
    cJSON *res;

    if (!error && animaStatus)
    {
        animationOn=1;
        res=create_result_text("animation started");

    }
    else if (!error && !animaStatus)
    {
        animationOn=0;
        res=create_result_text("animation stopped");
    }
    else 
    {
        res=create_result_text("unknown value");
    }

    return res;
}

static cJSON *tool_set_geometry(cJSON *args)
{
    const cJSON *status = cJSON_GetObjectItemCaseSensitive(args, "geometry");
    const char *geom = (cJSON_IsString(status) && status->valuestring) ? status->valuestring : "";
    cJSON *res;

    if (strcmp(geom,"disk")==0)
    {
        geometryType=0;
        res=create_result_text("geometry changed to disk");

    }
    else if (strcmp(geom,"plane")==0)
    {
        geometryType=1;
        res=create_result_text("geometry changed to plane");
    }
    else 
    {
        res=create_result_text("unknown value");
    }

    return res;
}

static cJSON *tool_set_symmetry(cJSON *args)
{
    const cJSON *status = cJSON_GetObjectItemCaseSensitive(args, "symmetry");
    const int sym = (cJSON_IsNumber(status) && status->valueint) ? status->valueint : 0;
    cJSON *res;

    if ((sym >= 0) && (sym < 3))
    {
        symmetry=sym;
        res=create_result_text("symmetry changed");

    }
    else 
    {
        res=create_result_text("unknown value");
    }

    return res;
}

static cJSON *tool_set_edge_color(cJSON *args)
{
    const cJSON *color = cJSON_GetObjectItemCaseSensitive(args, "color");
    const char *col = (cJSON_IsString(color) && color->valuestring) ? color->valuestring : "";

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

static cJSON *tool_set_background_color(cJSON *args)
{
    const cJSON *color = cJSON_GetObjectItemCaseSensitive(args, "color");
    const char *col = (cJSON_IsString(color) && color->valuestring) ? color->valuestring : "";

    int err = setColor(col,&backgroundColor);
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
    if (strcmp(name->valuestring, "edgeColor") == 0)
    {
        result = tool_set_edge_color((cJSON *)arguments);
    }
    else if (strcmp(name->valuestring, "backgroundColor") == 0)
    {
        result = tool_set_background_color((cJSON *)arguments);
    }
    else if (strcmp(name->valuestring, "animationOn") == 0)
    {
        result = tool_set_animation((cJSON *)arguments);
    }
    else if (strcmp(name->valuestring, "geometryType") == 0)
    {
        result = tool_set_geometry((cJSON *)arguments);
    }
    else if (strcmp(name->valuestring, "symmetryType") == 0)
    {
        result = tool_set_symmetry((cJSON *)arguments);
    }
    else if (strcmp(name->valuestring, "reset") == 0)
    {
        result = tool_reset((cJSON *)arguments);
    }
    //else if (strcmp(name->valuestring, "add") == 0)
    //{
    //    result = tool_add((cJSON *)arguments);
    //}
    else
    {
        return err(id, MCP_METHOD_NOT_FOUND, "Unknown tool");
    }
    return ok(id, result);
}

void define_tools()
{

    setDefaults();

    struct tool *tool = add_tool("edgeColor", "Set color of edges");
    add_argument(tool, 
        "color", TYPE_STR, "Color name (red, green, blue, white,black,gray)");

    tool = add_tool("backgroundColor", "Set color of background");
    add_argument(tool, 
        "color", TYPE_STR, "Color name (red, green, blue, white,black,gray)");

    tool = add_tool("animationOn", "start or stop animation");
    add_argument(tool, 
        "on", TYPE_BOOL, "Animation status");

    tool = add_tool("geometryType", "Type of geometry");
    add_argument(tool, 
        "geometry", TYPE_STR, "Type of geometry (disk or plane)");

    tool = add_tool("symmetryType", "Set the symmetry type");
    add_argument(tool, 
        "symmetry", TYPE_INT, "Type of symmetry (0,1,2)");

    tool = add_tool("reset", "Reset defaukt settings");
    
    //struct tool *addTool = add_tool("add", "Add two numbers");
    // Add arguments in reverse order (linked list)
    //add_argument(addTool, "b", TYPE_FLOAT, "Second number");
    //add_argument(addTool, "a", TYPE_FLOAT, "First number");
}