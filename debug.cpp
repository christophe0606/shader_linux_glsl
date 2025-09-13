extern "C" {
#include "cJSON.h"
#include "config.h"
#include "http.h"
#include "stdio_transport.h"
}

#include "tools.h"

#include <stdio.h>
#include <signal.h>

static volatile sig_atomic_t g_stop = 0;

void on_sigint(int sig)
{
    (void)sig;
    g_stop = 1;
}

int main() {

    signal(SIGINT, on_sigint);

    define_tools();

    int err=0;
#if defined(MCP_STDIO)
    err=init_stdio();
#else
    err=init_http();
#endif

   if (err != 0)
   {
         fprintf(stderr, "Failed to initialize MCP client\n");
         return 1;
   }

   while (!g_stop)
   {
#if defined(MCP_STDIO)
        process_stdio();
#else
        process_http();
#endif
   }

#if defined(MCP_STDIO)
    end_stdio();
#else
    end_http();
#endif

    return 0;
}
