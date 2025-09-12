#include "video.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include "hyperbolic.h"

std::string readShaderFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error opening shader file: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

GLuint createShaderProgram(const char *vertexPath, const char *fragmentPath)
{
    std::string vertexCode = readShaderFile(vertexPath);
    std::string fragmentCode = readShaderFile(fragmentPath);

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    GLuint vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    GLint success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "Error compiling vertex shader: " << infoLog << std::endl;
        return 0;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "Error compiling fragment shader: " << infoLog << std::endl;
        return 0;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shaderProgram;
}

GLuint mk_texture(int width, int height)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // For raw pixel buffers, nearest avoids filtering blur:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Clamp (or use GL_REPEAT if you want wraparound)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // If your rows aren’t 4-byte aligned (common with 3-channel RGB),
    // set unpack alignment to 1 to avoid row padding issues:
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Allocate storage once; no data yet. Replace width/height.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void gl_error_callback(int c, const char *d)
{
    fprintf(stderr, "GLFW error %d: %s\n", c, d);
}

static volatile sig_atomic_t g_stop = 0;

void on_sigint(int sig)
{
    (void)sig;
    g_stop = 1;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

int width, height;

int main(int argc, char **argv)
{
    signal(SIGINT, on_sigint);

    const char *dev_name = (argc > 1) ? argv[1] : "/dev/video5";
    int video_width = (argc > 2) ? atoi(argv[2]) : 640;
    int video_height = (argc > 3) ? atoi(argv[3]) : 480;

    enum v4l2_buf_type type;

    size_t rgb_size = (size_t)video_width * video_height * 3;
    unsigned char *rgb = (unsigned char *)malloc(rgb_size);

    if (!rgb)
    {
        fprintf(stderr, "malloc rgb failed\n");
        return 1;
    }
    memset(rgb, 0, rgb_size);

    int err = init_video(dev_name, video_width, video_height, type);
    if (err != 0)
        return err;

    

    
    glfwSetErrorCallback(gl_error_callback);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Fullscreen Example", monitor, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Set key callback
    glfwSetKeyCallback(window, key_callback);

    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, int w, int h)
                                   { glViewport(0, 0, w, h); 
                                width = w; height = h; });

    GLuint shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");
    if (shaderProgram == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    GLuint tex = mk_texture(video_width, video_height);

    GLint uResolutionLoc =  glGetUniformLocation(shaderProgram, "uResolution");
    GLint uTimeLoc = glGetUniformLocation(shaderProgram, "uTime");

    GLint uN1Loc = glGetUniformLocation(shaderProgram, "uN1");
    GLint uN2Loc = glGetUniformLocation(shaderProgram, "uN2");
    GLint uN3Loc = glGetUniformLocation(shaderProgram, "uN3");
    GLint uAALoc = glGetUniformLocation(shaderProgram, "uAA");
    GLint uTextureZoomLoc = glGetUniformLocation(shaderProgram, "uTextureZoom");

    vec3 n1, n2, n3;
    float zoom = 1.0f;
    //.compute_triangle(2,4,7, n1, n2, n3);
    //zoom = 0.5;
    compute_triangle(2,4,5, n1, n2, n3);
    zoom = 1.0;
    //compute_triangle(4,4,4, n1, n2, n3);
    //zoom = 0.5;

    float vertices[] = {
        // positions
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f};
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    float c = 0.0f;
    float dc = 0.1;
    double last = glfwGetTime();

    while (!glfwWindowShouldClose(window) && !g_stop)
    {
        double now = glfwGetTime();
        double dt = now - last;

        err = try_get_buffer(rgb, video_width, video_height);
        if (err != 0)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            continue;
        }



        glClear(GL_COLOR_BUFFER_BIT);

        // Update texture if the buffer changed (fast path: glTexSubImage2D)
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // keep safe
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height,
                        GL_RGB, GL_UNSIGNED_BYTE, rgb);

        // Bind texture unit 0 and set the sampler uniform
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        glUseProgram(shaderProgram);

        glUniform1i(glGetUniformLocation(shaderProgram, "uTex"), 0);

        glUniform2f(uResolutionLoc, (float)width, (float)height);    GLint uVideoWidthLoc = glGetUniformLocation(shaderProgram, "uVideoWidth");

        glUniform1f(uVideoWidthLoc, (float)video_width);
        glUniform1f(uTimeLoc, (float)now);

        glUniform3f(uN1Loc, (float)n1.x, (float)n1.y, (float)n1.z);
        glUniform3f(uN2Loc, (float)n2.x, (float)n2.y, (float)n2.z);
        glUniform3f(uN3Loc, (float)n3.x, (float)n3.y, (float)n3.z);

        glUniform1i(uAALoc, 4);
        glUniform1f(uTextureZoomLoc, (float)zoom);


        /*
        if (dt > 0.1) // update color every 0.1 seconds
        {
            c += dc;
            if (c > 1.0f || c < 0.0f)
                dc = -dc; // reverse direction
            last = now;
        }
            */
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    stop_video(type);


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    

    free(rgb);

    return 0;
}