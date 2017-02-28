/**
 * @file plugin.c
 * @brief Simple embedding python support
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-04-07
 */

/*
 * Ref:
 * - https://docs.python.org/3/c-api/init.html#thread-state-and-the-global-interpreter-lock
 * - http://www.slideshare.net/YiLungTsai/embed-python
 *
 */

// #define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <Python.h>

#include "i18n.h"
#include "meta.h"
#include "log.h"

#define PLUGIN_COUNT    2
#define PLUGIN_UPLOAD   0
#define PLUGIN_AVATAR   1

typedef struct {
    const char *mod_name;
    const char *func_name;
    PyObject *func;
} Plugin;

static PyThreadState *save;
/* Hash table to make sure that only one avatar query for one nick */
static GHashTable *avatars_hash;

Plugin plugins[PLUGIN_COUNT] = {
    {"upload", "upload", NULL},
    {"avatar", "avatar", NULL},
};

void plugin_init(){
    PyObject *py_name;
    PyObject *py_module;
    PyObject *py_func;

    Py_Initialize();
    PyEval_InitThreads();

    PyRun_SimpleString("import sys");

    GString *plugin_path = g_string_new("");
    g_string_append_printf(plugin_path, "sys.path.append('%s');",
            PACKAGE_DATA_DIR "/share/" PACKAGE "/plugins");
    g_string_append_printf(plugin_path, "sys.path.append('%s/srain/plugins');",
            g_get_user_config_dir());

    PyRun_SimpleString(plugin_path->str);

    g_string_free(plugin_path, TRUE);

    int i;
    for (i = 0; i < PLUGIN_COUNT; i++){
        /* Load plugin `upload' */
        py_name = PyUnicode_FromString(plugins[i].mod_name);

        py_module = PyImport_Import(py_name);
        Py_DECREF(py_name);

        if (!py_module) {
            PyErr_Print();
            LOG_FR("Plugin `%s` no found", plugins[i].mod_name);
            continue;
        }

        py_func = PyObject_GetAttrString(py_module, plugins[i].func_name);
        Py_DECREF(py_module);

        if (!py_func || !PyCallable_Check(py_func)) {
            PyErr_Print();
            WARN_FR("Function `%s` no found in module `%s`",
                    plugins[i].func_name, plugins[i].mod_name);
            continue;
        }

        LOG_FR("Plugin `%s` loaded", plugins[i].mod_name);

        plugins[i].func = py_func;
    }

    save = PyEval_SaveThread();

    avatars_hash = g_hash_table_new(g_str_hash, g_str_equal);
}

void plugin_finalize(){
    int i;

    for (i = 0; i < PLUGIN_COUNT; i++){
        if (plugins[i].func){
            Py_DECREF(plugins[i].func);
        }
    }

    g_hash_table_destroy(avatars_hash);

    PyEval_RestoreThread(save);
    Py_Finalize();
}

/**
 * @brief Given a file path, upload it to internet
 *
 * @param path
 *
 * @return If successed, return a URL of the uploaded file,
 *         If plugin not loaded or some error occured, return NULL.
 *
 * NOTE: This function is blocked.
 */
char* plugin_upload(const char *path){
    char *url;
    PyObject *py_args;
    PyObject *py_path;
    PyObject *py_url;

    if (!plugins[PLUGIN_UPLOAD].func){
        ERR_FR("Plugin no loaded");
        return NULL;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();

    url = NULL;
    /* Build arguments */
    py_path = PyUnicode_DecodeFSDefault(path);
    if (!py_path){
        PyErr_Print();
        ERR_FR("Failed to convert %s", path);
        goto bad;
    }

    py_args = PyTuple_New(1);
    PyTuple_SetItem(py_args, 0, py_path);

    /* Call python function */
    py_url = PyObject_CallObject(plugins[PLUGIN_UPLOAD].func, py_args);
    Py_DECREF(py_args);

    if (!py_url){
        PyErr_Print();
        ERR_FR("Function `%s` failed", plugins[PLUGIN_UPLOAD].func_name);
        goto bad;
    }

    url = PyUnicode_AsUTF8(py_url);

    if (!url){
        PyErr_Print();
        Py_DECREF(py_url);
        ERR_FR("Failed to convert URL to UTF-8");
        goto bad;
    }

    url = g_strdup(url);

    Py_DECREF(py_url);

bad:
    PyGILState_Release(gstate);
    return url;
}

typedef struct {
    char *nick;
    char *token;
} PluginAvatarData;

int _plugin_avatar(PluginAvatarData *data){
    int ret;
    char *path;
    const char *nick;
    const char *token;
    PyObject *py_nick;
    PyObject *py_token;
    PyObject *py_path;
    PyObject *py_args;

    nick = data->nick;
    token = data->token;
    path = g_build_filename(g_get_user_cache_dir(), PACKAGE, "avatars", NULL);

    ret = -1;

    if (!plugins[PLUGIN_AVATAR].func) return -1;

    PyGILState_STATE gstate = PyGILState_Ensure();

    if (!nick || !token || !path) goto bad;

    /* Build arguments */
    py_nick = PyUnicode_FromString(nick);
    if (!py_nick) goto bad;

    py_token = PyUnicode_FromString(token);
    if (!py_token) {
        Py_DECREF(py_nick);
        goto bad;
    }

    py_path = PyUnicode_FromString(path);
    if (!py_path) {
        Py_DECREF(py_nick);
        Py_DECREF(py_token);
        goto bad;
    }

    py_args = PyTuple_New(3);
    PyTuple_SetItem(py_args, 0, py_nick);
    PyTuple_SetItem(py_args, 1, py_token);
    PyTuple_SetItem(py_args, 2, py_path);

    PyObject_CallObject(plugins[PLUGIN_AVATAR].func, py_args);
    PyErr_Print();
    Py_DECREF(py_args);

    ret = 0;
bad:
    g_free(data->nick);
    g_free(data->token);
    g_free(path);
    g_free(data);
    PyGILState_Release(gstate);
    return ret;
}

/**
 * @brief Download `nick`'s avatar according to `token`
 *
 * @param nick
 * @param token
 *
 * @return 0
 *
 * NOTE: This function is async, always return 0
 */

int plugin_avatar_has_queried(const char *nick){
    if (g_hash_table_lookup(avatars_hash, nick)){
        DBG_FR("%s has queried", nick);
        return 1;
    } else {
        DBG_FR("%s hasn't queried yet", nick);
        g_hash_table_insert(avatars_hash, (char *)nick, (void *)1);
        return 0;
    }
}

int plugin_avatar(const char *nick, const char *token){
    PluginAvatarData *data;

    data = g_malloc0(sizeof(PluginAvatarData));
    data->nick = g_strdup(nick);
    data->token = g_strdup(token);

    g_thread_new(NULL, (GThreadFunc)_plugin_avatar, data);

    return 0;
}
