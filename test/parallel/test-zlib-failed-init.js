'use strict';

const common = require('../common');

const assert = require('assert');
const zlib = require('zlib');

common.expectsError(
  () => zlib.createGzip({ chunkSize: 0 }),
  {
    code: 'ERR_OUT_OF_RANGE',
    type: RangeError,
    message: 'The value of "options.chunkSize" is out of range. It must ' +
             'be >= 64. Received 0'
  }
);

common.expectsError(
  () => zlib.createGzip({ windowBits: 0 }),
  {
    code: 'ERR_OUT_OF_RANGE',
    type: RangeError,
    message: 'The value of "options.windowBits" is out of range. It must ' +
             'be >= 8 and <= 15. Received 0'
  }
);

common.expectsError(
  () => zlib.createGzip({ memLevel: 0 }),
  {
    code: 'ERR_OUT_OF_RANGE',
    type: RangeError,
    message: 'The value of "options.memLevel" is out of range. It must ' +
             'be >= 1 and <= 9. Received 0'
  }
);

{
  const stream = zlib.createGzip({ level: NaN });
  assert.strictEqual(stream._level, zlib.constants.Z_DEFAULT_COMPRESSION);
}

{
  const stream = zlib.createGzip({ strategy: NaN });
  assert.strictEqual(stream._strategy, zlib.constants.Z_DEFAULT_STRATEGY);
}
