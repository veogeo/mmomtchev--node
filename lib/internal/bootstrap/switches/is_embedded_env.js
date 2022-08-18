// This is the bootstrapping code used when creating a new environment
// through N-API

// Set up global.require and global.import so that they can
// be easily accessed from C/C++

const CJSLoader = require('internal/modules/cjs/loader');
const ESMLoader = require('internal/modules/esm/loader').ESMLoader;
const { StringDecoder } = require('string_decoder');
const { Writable } = require('stream');

global.module = new CJSLoader.Module();
global.require = require('module').createRequire(path);

const internalLoader = new ESMLoader();
const parent_path = require('url').pathToFileURL(path);
global.import = (mod) => internalLoader.import(mod, parent_path, Object.create(null));
global.import.meta = { url: parent_path };

class Stdout extends Writable {
  constructor(options) {
    super(options);
    this._decoder = new StringDecoder(options && options.defaultEncoding);
    this._handler = options.handler;
  }
  _write(chunk, encoding, callback) {
    if (encoding === 'buffer') {
      chunk = this._decoder.write(chunk);
    }
    this._handler(chunk);
    callback();
  }
  _final(callback) {
    this._handler(this._decoder.end());
    callback();
  }
}

// Set up stdio handlers if the caller is using custom callbacks
if (stdio) {
  if (stdio.stdin) {
    delete process.stdin;
    const { Readable } = require('stream');
    process.stdin = new Readable({ read() { } });
    process.stdin.push(null);
  }

  if (stdio.stdout) {
    delete process.stdout;
    process.stdout = new Stdout({
      handler: stdio.stdout
    });
  }

  if (stdio.stderr) {
    delete process.stderr;
    process.stderr = new Stdout({
      handler: stdio.stderr
    });
  }
}
