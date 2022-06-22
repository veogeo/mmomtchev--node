#define NAPI_EXPERIMENTAL
#include <assert.h>
#include <node_api.h>

#include <stdio.h>

// Note: This file is being referred to from doc/api/embedding.md, and excerpts
// from it are included in the documentation. Try to keep these in sync.

static int RunNodeInstance(napi_platform platform);

const char* main_script =
    "const publicRequire ="
    "  require('module').createRequire(process.cwd() + '/');"
    "globalThis.require = publicRequire;"
    "globalThis.embedVars = { nön_ascıı: '🏳️‍🌈' };"
    "require('vm').runInThisContext(process.argv[1]);";

int main(int argc, char** argv) {
  napi_platform platform;
  napi_status r;

  r = napi_create_platform(argc, argv, 0, NULL, NULL, 0, &platform);
  if (r != napi_ok) {
    fprintf(stderr, "Failed creating the platform\n");
    return -1;
  }

  int exit_code = RunNodeInstance(platform);

  napi_destroy_platform(platform);
  if (r != napi_ok) {
    fprintf(stderr, "Failed destroying the platform\n");
    return -1;
  }

  return exit_code;
}

int RunNodeInstance(napi_platform platform) {
  napi_status r;
  napi_env env;
  int exit_code;

  r = napi_create_environment(platform, NULL, main_script, &env);
  if (r != napi_ok) {
    fprintf(stderr, "Failed running JS\n");
    return -1;
  }

  r = napi_destroy_environment(env, &exit_code);
  if (r != napi_ok) return -1;
  return exit_code;
}
