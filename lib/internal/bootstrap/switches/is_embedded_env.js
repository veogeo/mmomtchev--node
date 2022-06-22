// This is the bootstrapping code used when creating a new environment
// through N-API

// Set up global.require and global.import so that they can
// be easily accessed from C/C++

const CJSLoader = require('internal/modules/cjs/loader');
const ESMLoader = require('internal/modules/esm/loader').ESMLoader;

global.module = new CJSLoader.Module();
global.require = require('module').createRequire(path);

const internalLoader = new ESMLoader();
const parent_path = require('url').pathToFileURL(path);
global.import = (mod) => internalLoader.import(mod, parent_path, Object.create(null));
global.import.meta = { url: parent_path };
