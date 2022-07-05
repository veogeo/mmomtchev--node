#define NAPI_EXPERIMENTAL
#include <assert.h>
#include <node_api.h>

#include <stdio.h>
#include <string.h>

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

int callMe(napi_env env) {
  napi_handle_scope scope;
  napi_value global;
  napi_value cb;
  napi_value key;

  napi_open_handle_scope(env, &scope);

  if (napi_get_global(env, &global) != napi_ok) {
    fprintf(stderr, "Failed accessing the global object\n");
    return -1;
  }

  napi_create_string_utf8(env, "callMe", strlen("callMe"), &key);

  if (napi_get_property(env, global, key, &cb) != napi_ok) {
    fprintf(stderr, "Failed accessing the global object\n");
    return -1;
  }

  napi_valuetype cb_type;
  if (napi_typeof(env, cb, &cb_type) != napi_ok) {
    fprintf(stderr, "Failed accessing the global object\n");
    return -1;
  }

  if (cb_type == napi_function) {
    napi_value undef;
    napi_get_undefined(env, &undef);
    napi_value arg;
    napi_create_string_utf8(env, "called", strlen("called"), &arg);
    napi_value result;
    napi_call_function(env, undef, cb, 1, &arg, &result);

    char buf[16];
    size_t len;
    napi_get_value_string_utf8(env, result, buf, 16, &len);
    if (strncmp(buf, "called you", len)) {
      fprintf(stderr, "Invalid value received: %s\n", buf);
      return -1;
    }
    printf("%s", buf);
  } else if (cb_type != napi_undefined) {
    fprintf(stderr, "Invalid callMe value\n");
    return -1;
  }

  napi_value object;
  if (napi_create_object(env, &object) != napi_ok) {
    fprintf(stderr, "Failed creating an object\n");
    return -1;
  }

  napi_close_handle_scope(env, scope);
  return 0;
}

char callback_buf[16];
size_t callback_buf_len;
napi_value c_cb(napi_env env, napi_callback_info info) {
  napi_handle_scope scope;
  size_t argc = 1;
  napi_value arg;
  napi_value undef;

  napi_open_handle_scope(env, &scope);
  napi_get_cb_info(env, info, &argc, &arg, NULL, NULL);

  napi_get_value_string_utf8(env, arg, callback_buf, 16, &callback_buf_len);
  napi_get_undefined(env, &undef);
  napi_close_handle_scope(env, scope);
  return undef;
}

int waitMe(napi_env env) {
  napi_handle_scope scope;
  napi_value global;
  napi_value cb;
  napi_value key;

  napi_open_handle_scope(env, &scope);

  if (napi_get_global(env, &global) != napi_ok) {
    fprintf(stderr, "Failed accessing the global object\n");
    return -1;
  }

  napi_create_string_utf8(env, "waitMe", strlen("waitMe"), &key);

  if (napi_get_property(env, global, key, &cb) != napi_ok) {
    fprintf(stderr, "Failed accessing the global object\n");
    return -1;
  }

  napi_valuetype cb_type;
  if (napi_typeof(env, cb, &cb_type) != napi_ok) {
    fprintf(stderr, "Failed accessing the global object\n");
    return -1;
  }

  if (cb_type == napi_function) {
    napi_value undef;
    napi_get_undefined(env, &undef);
    napi_value args[2];
    napi_create_string_utf8(env, "waited", strlen("waited"), &args[0]);
    if (napi_create_function(
            env, "wait_cb", strlen("wait_cb"), c_cb, NULL, &args[1]) !=
        napi_ok) {
      fprintf(stderr, "Failed creating function\n");
      return -1;
    };

    napi_value result;
    napi_call_function(env, undef, cb, 2, args, &result);
    if (!strncmp(callback_buf, "waited you", strlen("waited you"))) {
      fprintf(stderr, "Anachronism detected: %s\n", callback_buf);
      return -1;
    }

    if (napi_run_environment(env) != napi_ok) {
      fprintf(stderr, "Failed spinning the event loop\n");
      return -1;
    }

    if (strncmp(callback_buf, "waited you", callback_buf_len)) {
      fprintf(stderr, "Invalid value received: %s\n", callback_buf);
      return -1;
    }
    printf("%s", callback_buf);
  } else if (cb_type != napi_undefined) {
    fprintf(stderr, "Invalid waitMe value\n");
    return -1;
  }

  napi_close_handle_scope(env, scope);
  return 0;
}

int RunNodeInstance(napi_platform platform) {
  napi_status r;
  napi_env env;
  int exit_code;

  if (napi_create_environment(platform, NULL, main_script, &env) != napi_ok) {
    fprintf(stderr, "Failed running JS\n");
    return -1;
  }

  if (callMe(env) != 0) return -1;
  if (waitMe(env) != 0) return -1;

  r = napi_destroy_environment(env, &exit_code);
  if (r != napi_ok) return -1;
  return exit_code;
}
