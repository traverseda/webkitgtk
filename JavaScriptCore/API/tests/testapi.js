/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

function bludgeonArguments() { if (0) arguments; return function g() {} }
h = bludgeonArguments();
gc();

var failed = false;
function pass(msg)
{
    print("PASS: " + msg, "green");
}

function fail(msg)
{
    print("FAIL: " + msg, "red");
    failed = true;
}

function shouldBe(a, b)
{
    var evalA;
    try {
        evalA = eval(a);
    } catch(e) {
        evalA = e;
    }
    
    if (evalA == b || isNaN(evalA) && typeof evalA == 'number' && isNaN(b) && typeof b == 'number')
        pass(a + " should be " + b + " and is.");
    else
        fail(a + " should be " + b + " but instead is " + evalA + ".");
}

function shouldThrow(a)
{
    var evalA;
    try {
        eval(a);
    } catch(e) {
        pass(a + " threw: " + e);
        return;
    }

    fail(a + " did not throw an exception.");
}

function globalStaticFunction()
{
    return 4;
}

shouldBe("globalStaticValue", 3);
shouldBe("globalStaticFunction()", 4);

shouldBe("typeof MyObject", "function"); // our object implements 'call'
MyObject.cantFind = 1;
shouldBe("MyObject.cantFind", undefined);
MyObject.regularType = 1;
shouldBe("MyObject.regularType", 1);
MyObject.alwaysOne = 2;
shouldBe("MyObject.alwaysOne", 1);
MyObject.cantDelete = 1;
delete MyObject.cantDelete;
shouldBe("MyObject.cantDelete", 1);
shouldBe("delete MyObject.throwOnDelete", "an exception");
MyObject.cantSet = 1;
shouldBe("MyObject.cantSet", undefined);
shouldBe("MyObject.throwOnGet", "an exception");
shouldBe("MyObject.throwOnSet = 5", "an exception");
shouldBe("MyObject('throwOnCall')", "an exception");
shouldBe("new MyObject('throwOnConstruct')", "an exception");
shouldBe("'throwOnHasInstance' instanceof MyObject", "an exception");

var foundMyPropertyName = false;
var foundRegularType = false;
for (var p in MyObject) {
    if (p == "myPropertyName")
        foundMyPropertyName = true;
    if (p == "regularType")
        foundRegularType = true;
}

if (foundMyPropertyName)
    pass("MyObject.myPropertyName was enumerated");
else
    fail("MyObject.myPropertyName was not enumerated");

if (foundRegularType)
    pass("MyObject.regularType was enumerated");
else
    fail("MyObject.regularType was not enumerated");

myObject = new MyObject();

shouldBe("delete MyObject.regularType", true);
shouldBe("MyObject.regularType", undefined);
shouldBe("MyObject(0)", 1);
shouldBe("MyObject()", undefined);
shouldBe("typeof myObject", "object");
shouldBe("MyObject ? 1 : 0", true); // toBoolean
shouldBe("+MyObject", 1); // toNumber
shouldBe("(MyObject.toString())", "[object MyObject]"); // toString
shouldBe("String(MyObject)", "MyObjectAsString"); // type conversion to string
shouldBe("MyObject - 0", 1); // toNumber

shouldBe("typeof MyConstructor", "object");
constructedObject = new MyConstructor(1);
shouldBe("typeof constructedObject", "object");
shouldBe("constructedObject.value", 1);
shouldBe("myObject instanceof MyObject", true);
shouldBe("(new Object()) instanceof MyObject", false);

shouldThrow("MyObject.nullGetSet = 1");
shouldThrow("MyObject.nullGetSet");
shouldThrow("MyObject.nullCall()");
shouldThrow("MyObject.hasPropertyLie");

derived = new Derived();

// base properties and functions return 1 when called/gotten; derived, 2
shouldBe("derived.baseProtoDup()", 2);
shouldBe("derived.baseProto()", 1);
shouldBe("derived.baseDup", 2);
shouldBe("derived.baseOnly", 1);
shouldBe("derived.protoOnly()", 2);
shouldBe("derived.protoDup", 2);
shouldBe("derived.derivedOnly", 2)

// base properties throw 1 when set; derived, 2
shouldBe("derived.baseDup = 0", 2);
shouldBe("derived.baseOnly = 0", 1);
shouldBe("derived.derivedOnly = 0", 2)
shouldBe("derived.protoDup = 0", 2);

shouldBe("undefined instanceof MyObject", false);
EvilExceptionObject.hasInstance = function f() { return f(); };
EvilExceptionObject.__proto__ = undefined;
shouldThrow("undefined instanceof EvilExceptionObject");
EvilExceptionObject.hasInstance = function () { return true; };
shouldBe("undefined instanceof EvilExceptionObject", true);

EvilExceptionObject.toNumber = function f() { return f(); }
shouldThrow("EvilExceptionObject*5");
EvilExceptionObject.toStringExplicit = function f() { return f(); }
shouldThrow("String(EvilExceptionObject)");

shouldBe("EmptyObject", "[object CallbackObject]");

if (failed)
    throw "Some tests failed";

