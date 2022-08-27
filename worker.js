const {
  Worker,
  isMainThread
} = require('worker_threads');

if (isMainThread) {
  const worker = new Worker(__filename);
  console.log('main');
} else {
  console.log('worker');
}
