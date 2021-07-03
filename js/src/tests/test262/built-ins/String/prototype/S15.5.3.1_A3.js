// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
info: The String.prototype property has the attribute DontDelete
es5id: 15.5.3.1_A3
description: Checking if deleting the String.prototype property fails
includes: [propertyHelper.js]
---*/

//////////////////////////////////////////////////////////////////////////////
//CHECK#1
if (!(String.hasOwnProperty('prototype'))) {
  $ERROR('#1: String.hasOwnProperty(\'prototype\') return true. Actual: ' + String.hasOwnProperty('prototype'));
}
//
//////////////////////////////////////////////////////////////////////////////

verifyNotConfigurable(String, "prototype");

//////////////////////////////////////////////////////////////////////////////
//CHECK#2
try {
  if ((delete String.prototype) !== false) {
    $ERROR('#2: String.prototype has the attribute DontDelete');
  }
} catch (e) {
  if (e instanceof Test262Error) throw e;
  assert(e instanceof TypeError);
}
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//CHECK#3
if (!(String.hasOwnProperty('prototype'))) {
  $ERROR('#3: delete String.prototype; String.hasOwnProperty(\'prototype\') return true. Actual: ' + String.hasOwnProperty('prototype'));
}
//
//////////////////////////////////////////////////////////////////////////////

reportCompare(0, 0);