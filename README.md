# README

The `hyperbolic` app displays a fullscreen shader with a video texture coming from camera. It is a tiling of the hyperbolic plane. It can be controlled from a LLM through MCP protocol (the app is an MCP server).

![Hyperbolic screenshot](screenshot.png "Screenshot")

The tiling is defined by a group of symmetries and they can be changed.

The hyperbolic plane can either be represented as a disk or as a plane:

![Hyperbolic screenshot with plane geometry](screenshot_plane.png "Screenshot with plane geometry")

The video device can be passed as argument when launching the app:

`hyperbolic /dev/video5`

The app needs to read the files `fragment_shader.glsl` and `vertex_shader.glsl`. They should be in the working directory. You should run the app from the root folder otherwise the files won't be found by the app:

`build\Debug\hyperbolic`

The app exposes an MCP server on port 8100 and can be controlled by a LLM. Here is an example of a chat to change the settings of the hyperbolic app:


![MCP example](mcp_screenshot.png "MCP session example")

To use the app with a LLM you need:

* A LLM server. The `server.sh` shows how to laumch `llama-server` from `llama.cpp` project. An Open AI API compatible server will work
* A chat client that can talk to the open AI compatible interface and to MCP servers. For instance [Jan](https://jan.ai)

Typical commands provided by the tool:

* `change background to black`
* `change edge to green`
* `stop animation`
* `change geometry to plane`
* `change symmetry to 1`
* `reset settings`



## To build 

`git clone --recurse-submodules https://github.com/christophe0606/shader_linux_glsl.git`

Some libraries are required for the UI

```bash
sudo apt install libglew-dev
sudo apt install libglfw3-dev
sudo apt install libgles2-mesa-dev
``` 

If you don't want to build but just use the excutable:

```bash 
sudo apt install libglfw3
sudo apt install libglew2.2
sudo apt install libgles2
```

Then use `cmake` to build.

## AI

Except the shader that is coming from an old experiment by the author, other parts of this demo have been generated a lot by chat GPT.

