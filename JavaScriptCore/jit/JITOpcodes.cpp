/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JIT.h"

#if ENABLE(JIT)

#include "JITInlineMethods.h"
#include "JITStubCall.h"
#include "JSArray.h"
#include "JSCell.h"
#include "JSFunction.h"
#include "LinkBuffer.h"

namespace JSC {

#if USE(JSVALUE32_64)

void JIT::privateCompileCTIMachineTrampolines(RefPtr<ExecutablePool>* executablePool, JSGlobalData* globalData, CodePtr* ctiStringLengthTrampoline, CodePtr* ctiVirtualCallPreLink, CodePtr* ctiVirtualCallLink, CodePtr* ctiVirtualCall, CodePtr* ctiNativeCallThunk)
{
#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    // (1) This function provides fast property access for string length
    Label stringLengthBegin = align();
    
    // regT0 holds payload, regT1 holds tag
    
    Jump string_failureCases1 = branch32(NotEqual, regT1, Imm32(JSValue::CellTag));
    Jump string_failureCases2 = branchPtr(NotEqual, Address(regT0), ImmPtr(m_globalData->jsStringVPtr));

    // Checks out okay! - get the length from the Ustring.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSString, m_value) + OBJECT_OFFSETOF(UString, m_rep)), regT2);
    load32(Address(regT2, OBJECT_OFFSETOF(UString::Rep, len)), regT2);

    Jump string_failureCases3 = branch32(Above, regT2, Imm32(INT_MAX));
    move(regT2, regT0);
    move(Imm32(JSValue::Int32Tag), regT1);

    ret();
#endif

    // (2) Trampolines for the slow cases of op_call / op_call_eval / op_construct.

#if ENABLE(JIT_OPTIMIZE_CALL)
    /* VirtualCallPreLink Trampoline */
    Label virtualCallPreLinkBegin = align();

    // regT0 holds callee, regT1 holds argCount.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSFunction, m_body)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(FunctionBodyNode, m_code)), regT2);
    Jump hasCodeBlock1 = branchTestPtr(NonZero, regT2);

    // Lazily generate a CodeBlock.
    preserveReturnAddressAfterCall(regT3); // return address
    restoreArgumentReference();
    Call callJSFunction1 = call();
    move(regT0, regT2);
    emitGetJITStubArg(1, regT0); // callee
    emitGetJITStubArg(5, regT1); // argCount
    restoreReturnAddressBeforeReturn(regT3); // return address
    hasCodeBlock1.link(this);

    // regT2 holds codeBlock.
    Jump isNativeFunc1 = branch32(Equal, Address(regT2, OBJECT_OFFSETOF(CodeBlock, m_codeType)), Imm32(NativeCode));

    // Check argCount matches callee arity.
    Jump arityCheckOkay1 = branch32(Equal, Address(regT2, OBJECT_OFFSETOF(CodeBlock, m_numParameters)), regT1);
    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 3); // return address
    emitPutJITStubArg(regT2, 7); // codeBlock
    restoreArgumentReference();
    Call callArityCheck1 = call();
    move(regT1, callFrameRegister);
    emitGetJITStubArg(1, regT0); // callee
    emitGetJITStubArg(5, regT1); // argCount
    restoreReturnAddressBeforeReturn(regT3); // return address

    arityCheckOkay1.link(this);
    isNativeFunc1.link(this);
    
    compileOpCallInitializeCallFrame();

    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 3);
    restoreArgumentReference();
    Call callDontLazyLinkCall = call();
    restoreReturnAddressBeforeReturn(regT3);
    jump(regT0);

    /* VirtualCallLink Trampoline */
    Label virtualCallLinkBegin = align();

    // regT0 holds callee, regT1 holds argCount.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSFunction, m_body)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(FunctionBodyNode, m_code)), regT2);
    Jump hasCodeBlock2 = branchTestPtr(NonZero, regT2);

    // Lazily generate a CodeBlock.
    preserveReturnAddressAfterCall(regT3); // return address
    restoreArgumentReference();
    Call callJSFunction2 = call();
    move(regT0, regT2);
    emitGetJITStubArg(1, regT0); // callee
    emitGetJITStubArg(5, regT1); // argCount
    restoreReturnAddressBeforeReturn(regT3); // return address
    hasCodeBlock2.link(this);

    // regT2 holds codeBlock.
    Jump isNativeFunc2 = branch32(Equal, Address(regT2, OBJECT_OFFSETOF(CodeBlock, m_codeType)), Imm32(NativeCode));

    // Check argCount matches callee arity.
    Jump arityCheckOkay2 = branch32(Equal, Address(regT2, OBJECT_OFFSETOF(CodeBlock, m_numParameters)), regT1);
    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 3); // return address
    emitPutJITStubArg(regT2, 7); // codeBlock
    restoreArgumentReference();
    Call callArityCheck2 = call();
    move(regT1, callFrameRegister);
    emitGetJITStubArg(1, regT0); // callee
    emitGetJITStubArg(5, regT1); // argCount
    restoreReturnAddressBeforeReturn(regT3); // return address

    arityCheckOkay2.link(this);
    isNativeFunc2.link(this);

    compileOpCallInitializeCallFrame();

    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 3);
    restoreArgumentReference();
    Call callLazyLinkCall = call();
    restoreReturnAddressBeforeReturn(regT3);
    jump(regT0);
#endif // ENABLE(JIT_OPTIMIZE_CALL)

    /* VirtualCall Trampoline */
    Label virtualCallBegin = align();

    // regT0 holds callee, regT1 holds argCount.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSFunction, m_body)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(FunctionBodyNode, m_code)), regT2);
    Jump hasCodeBlock3 = branchTestPtr(NonZero, regT2);

    // Lazily generate a CodeBlock.
    preserveReturnAddressAfterCall(regT3); // return address
    restoreArgumentReference();
    Call callJSFunction3 = call();
    move(regT0, regT2);
    emitGetJITStubArg(1, regT0); // callee
    emitGetJITStubArg(5, regT1); // argCount
    restoreReturnAddressBeforeReturn(regT3); // return address
    hasCodeBlock3.link(this);
    
    // regT2 holds codeBlock.
    Jump isNativeFunc3 = branch32(Equal, Address(regT2, OBJECT_OFFSETOF(CodeBlock, m_codeType)), Imm32(NativeCode));
    
    // Check argCount matches callee.
    Jump arityCheckOkay3 = branch32(Equal, Address(regT2, OBJECT_OFFSETOF(CodeBlock, m_numParameters)), regT1);
    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 3); // return address
    emitPutJITStubArg(regT2, 7); // codeBlock
    restoreArgumentReference();
    Call callArityCheck3 = call();
    move(regT1, callFrameRegister);
    emitGetJITStubArg(1, regT0); // callee
    emitGetJITStubArg(5, regT1); // argCount
    restoreReturnAddressBeforeReturn(regT3); // return address

    arityCheckOkay3.link(this);
    isNativeFunc3.link(this);
    compileOpCallInitializeCallFrame();
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSFunction, m_body)), regT0);
    loadPtr(Address(regT0, OBJECT_OFFSETOF(FunctionBodyNode, m_jitCode)), regT0);
    jump(regT0);

#if PLATFORM(X86)
    Label nativeCallThunk = align();
    preserveReturnAddressAfterCall(regT0);
    emitPutToCallFrameHeader(regT0, RegisterFile::ReturnPC); // Push return address

    // Load caller frame's scope chain into this callframe so that whatever we call can
    // get to its global data.
    emitGetFromCallFrameHeaderPtr(RegisterFile::CallerFrame, regT1);
    emitGetFromCallFrameHeaderPtr(RegisterFile::ScopeChain, regT1, regT1);
    emitPutToCallFrameHeader(regT1, RegisterFile::ScopeChain);
    
    emitGetFromCallFrameHeader32(RegisterFile::ArgumentCount, regT0);

    /* We have two structs that we use to describe the stackframe we set up for our
     * call to native code.  NativeCallFrameStructure describes the how we set up the stack
     * in advance of the call.  NativeFunctionCalleeSignature describes the callframe
     * as the native code expects it.  We do this as we are using the fastcall calling
     * convention which results in the callee popping its arguments off the stack, but
     * not the rest of the callframe so we need a nice way to ensure we increment the
     * stack pointer by the right amount after the call.
     */

#if COMPILER(MSVC) || PLATFORM(LINUX)
#if COMPILER(MSVC)
#pragma pack(push)
#pragma pack(4)
#endif // COMPILER(MSVC)
    struct NativeCallFrameStructure {
      //  CallFrame* callFrame; // passed in EDX
        JSObject* callee;
        JSValue thisValue;
        ArgList* argPointer;
        ArgList args;
        JSValue result;
    };
    struct NativeFunctionCalleeSignature {
        JSObject* callee;
        JSValue thisValue;
        ArgList* argPointer;
    };
#if COMPILER(MSVC)
#pragma pack(pop)
#endif // COMPILER(MSVC)
#else
    struct NativeCallFrameStructure {
      //  CallFrame* callFrame; // passed in ECX
      //  JSObject* callee; // passed in EDX
        JSValue thisValue;
        ArgList* argPointer;
        ArgList args;
    };
    struct NativeFunctionCalleeSignature {
        JSValue thisValue;
        ArgList* argPointer;
    };
#endif
    
    const int NativeCallFrameSize = (sizeof(NativeCallFrameStructure) + 15) & ~15;
    // Allocate system stack frame
    subPtr(Imm32(NativeCallFrameSize), stackPointerRegister);

    // Set up arguments
    subPtr(Imm32(1), regT0); // Don't include 'this' in argcount

    // push argcount
    storePtr(regT0, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, args) + OBJECT_OFFSETOF(ArgList, m_argCount)));
    
    // Calculate the start of the callframe header, and store in regT1
    addPtr(Imm32(-RegisterFile::CallFrameHeaderSize * (int)sizeof(Register)), callFrameRegister, regT1);
    
    // Calculate start of arguments as callframe header - sizeof(Register) * argcount (regT0)
    mul32(Imm32(sizeof(Register)), regT0, regT0);
    subPtr(regT0, regT1);
    storePtr(regT1, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, args) + OBJECT_OFFSETOF(ArgList, m_args)));

    // ArgList is passed by reference so is stackPointerRegister + 4 * sizeof(Register)
    addPtr(Imm32(OBJECT_OFFSETOF(NativeCallFrameStructure, args)), stackPointerRegister, regT0);
    storePtr(regT0, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, argPointer)));

    // regT1 currently points to the first argument, regT1 - sizeof(Register) points to 'this'
    loadPtr(Address(regT1, -(int)sizeof(Register) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), regT2);
    loadPtr(Address(regT1, -(int)sizeof(Register) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), regT3);
    storePtr(regT2, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, thisValue) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
    storePtr(regT3, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, thisValue) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));

#if COMPILER(MSVC) || PLATFORM(LINUX)
    // ArgList is passed by reference so is stackPointerRegister + 4 * sizeof(Register)
    addPtr(Imm32(OBJECT_OFFSETOF(NativeCallFrameStructure, result)), stackPointerRegister, X86::ecx);

    // Plant callee
    emitGetFromCallFrameHeaderPtr(RegisterFile::Callee, X86::eax);
    storePtr(X86::eax, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, callee)));

    // Plant callframe
    move(callFrameRegister, X86::edx);

    call(Address(X86::eax, OBJECT_OFFSETOF(JSFunction, m_data)));

    // JSValue is a non-POD type, so eax points to it
    emitLoad(0, regT1, regT0, X86::eax);
#else
    emitGetFromCallFrameHeaderPtr(RegisterFile::Callee, X86::edx); // callee
    move(callFrameRegister, X86::ecx); // callFrame
    call(Address(X86::edx, OBJECT_OFFSETOF(JSFunction, m_data)));
#endif

    // We've put a few temporaries on the stack in addition to the actual arguments
    // so pull them off now
    addPtr(Imm32(NativeCallFrameSize - sizeof(NativeFunctionCalleeSignature)), stackPointerRegister);

    // Check for an exception
    // FIXME: Maybe we can optimize this comparison to JSValue().
    move(ImmPtr(&globalData->exception), regT2);
    Jump sawException1 = branch32(NotEqual, tagFor(0, regT2), Imm32(JSValue::CellTag));
    Jump sawException2 = branch32(NonZero, payloadFor(0, regT2), Imm32(0));

    // Grab the return address.
    emitGetFromCallFrameHeaderPtr(RegisterFile::ReturnPC, regT3);
    
    // Restore our caller's "r".
    emitGetFromCallFrameHeaderPtr(RegisterFile::CallerFrame, callFrameRegister);
    
    // Return.
    restoreReturnAddressBeforeReturn(regT3);
    ret();

    // Handle an exception
    sawException1.link(this);
    sawException2.link(this);
    // Grab the return address.
    emitGetFromCallFrameHeaderPtr(RegisterFile::ReturnPC, regT1);
    move(ImmPtr(&globalData->exceptionLocation), regT2);
    storePtr(regT1, regT2);
    move(ImmPtr(reinterpret_cast<void*>(ctiVMThrowTrampoline)), regT2);
    emitGetFromCallFrameHeaderPtr(RegisterFile::CallerFrame, callFrameRegister);
    poke(callFrameRegister, OBJECT_OFFSETOF(struct JITStackFrame, callFrame) / sizeof (void*));
    restoreReturnAddressBeforeReturn(regT2);
    ret();

#elif ENABLE(JIT_OPTIMIZE_NATIVE_CALL)
#error "JIT_OPTIMIZE_NATIVE_CALL not yet supported on this platform."
#else
    breakpoint();
#endif
    
#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    Call string_failureCases1Call = makeTailRecursiveCall(string_failureCases1);
    Call string_failureCases2Call = makeTailRecursiveCall(string_failureCases2);
    Call string_failureCases3Call = makeTailRecursiveCall(string_failureCases3);
#endif

    // All trampolines constructed! copy the code, link up calls, and set the pointers on the Machine object.
    LinkBuffer patchBuffer(this, m_globalData->executableAllocator.poolForSize(m_assembler.size()));

#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    patchBuffer.link(string_failureCases1Call, FunctionPtr(cti_op_get_by_id_string_fail));
    patchBuffer.link(string_failureCases2Call, FunctionPtr(cti_op_get_by_id_string_fail));
    patchBuffer.link(string_failureCases3Call, FunctionPtr(cti_op_get_by_id_string_fail));
#endif
#if ENABLE(JIT_OPTIMIZE_CALL)
    patchBuffer.link(callArityCheck1, FunctionPtr(cti_op_call_arityCheck));
    patchBuffer.link(callJSFunction1, FunctionPtr(cti_op_call_JSFunction));
    patchBuffer.link(callArityCheck2, FunctionPtr(cti_op_call_arityCheck));
    patchBuffer.link(callJSFunction2, FunctionPtr(cti_op_call_JSFunction));
    patchBuffer.link(callDontLazyLinkCall, FunctionPtr(cti_vm_dontLazyLinkCall));
    patchBuffer.link(callLazyLinkCall, FunctionPtr(cti_vm_lazyLinkCall));
#endif
    patchBuffer.link(callArityCheck3, FunctionPtr(cti_op_call_arityCheck));
    patchBuffer.link(callJSFunction3, FunctionPtr(cti_op_call_JSFunction));

    CodeRef finalCode = patchBuffer.finalizeCode();
    *executablePool = finalCode.m_executablePool;

    *ctiVirtualCall = trampolineAt(finalCode, virtualCallBegin);
    *ctiNativeCallThunk = trampolineAt(finalCode, nativeCallThunk);
#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    *ctiStringLengthTrampoline = trampolineAt(finalCode, stringLengthBegin);
#else
    UNUSED_PARAM(ctiStringLengthTrampoline);
#endif
#if ENABLE(JIT_OPTIMIZE_CALL)
    *ctiVirtualCallPreLink = trampolineAt(finalCode, virtualCallPreLinkBegin);
    *ctiVirtualCallLink = trampolineAt(finalCode, virtualCallLinkBegin);
#else
    UNUSED_PARAM(ctiVirtualCallPreLink);
    UNUSED_PARAM(ctiVirtualCallLink);
#endif
}

void JIT::emit_op_mov(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src = currentInstruction[2].u.operand;

    if (m_codeBlock->isConstantRegisterIndex(src))
        emitStore(dst, getConstantOperand(src));
    else {
        emitLoad(src, regT1, regT0);
        emitStore(dst, regT1, regT0);
        map(m_bytecodeIndex + OPCODE_LENGTH(op_mov), dst, regT1, regT0);
    }
}

void JIT::emit_op_end(Instruction* currentInstruction)
{
    if (m_codeBlock->needsFullScopeChain())
        JITStubCall(this, cti_op_end).call();
    ASSERT(returnValueRegister != callFrameRegister);
    emitLoad(currentInstruction[1].u.operand, regT1, regT0);
    restoreReturnAddressBeforeReturn(Address(callFrameRegister, RegisterFile::ReturnPC * static_cast<int>(sizeof(Register))));
    ret();
}

void JIT::emit_op_jmp(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[1].u.operand;
    addJump(jump(), target + 1);
}

void JIT::emit_op_loop(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[1].u.operand;
    emitTimeoutCheck();
    addJump(jump(), target + 1);
}

void JIT::emit_op_loop_if_less(Instruction* currentInstruction)
{
    unsigned op1 = currentInstruction[1].u.operand;
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emitTimeoutCheck();

    if (isOperandConstantImmediateInt(op1)) {
        emitLoad(op2, regT1, regT0);
        addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag)));
        addJump(branch32(GreaterThan, regT0, Imm32(getConstantOperand(op1).asInt32())), target + 3);
        return;
    }
    
    if (isOperandConstantImmediateInt(op2)) {
        emitLoad(op1, regT1, regT0);
        addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag)));
        addJump(branch32(LessThan, regT0, Imm32(getConstantOperand(op2).asInt32())), target + 3);
        return;
    }

    emitLoad2(op1, regT1, regT0, op2, regT3, regT2);
    addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag)));
    addSlowCase(branch32(NotEqual, regT3, Imm32(JSValue::Int32Tag)));
    addJump(branch32(LessThan, regT0, regT2), target + 3);
}

void JIT::emitSlow_op_loop_if_less(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned op1 = currentInstruction[1].u.operand;
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    if (!isOperandConstantImmediateInt(op1) && !isOperandConstantImmediateInt(op2))
        linkSlowCase(iter); // int32 check
    linkSlowCase(iter); // int32 check

    JITStubCall stubCall(this, cti_op_loop_if_less);
    stubCall.addArgument(op1);
    stubCall.addArgument(op2);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 3);
}

void JIT::emit_op_loop_if_lesseq(Instruction* currentInstruction)
{
    unsigned op1 = currentInstruction[1].u.operand;
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emitTimeoutCheck();

    if (isOperandConstantImmediateInt(op1)) {
        emitLoad(op2, regT1, regT0);
        addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag)));
        addJump(branch32(GreaterThanOrEqual, regT0, Imm32(getConstantOperand(op1).asInt32())), target + 3);
        return;
    }

    if (isOperandConstantImmediateInt(op2)) {
        emitLoad(op1, regT1, regT0);
        addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag)));
        addJump(branch32(LessThanOrEqual, regT0, Imm32(getConstantOperand(op2).asInt32())), target + 3);
        return;
    }

    emitLoad2(op1, regT1, regT0, op2, regT3, regT2);
    addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag)));
    addSlowCase(branch32(NotEqual, regT3, Imm32(JSValue::Int32Tag)));
    addJump(branch32(LessThanOrEqual, regT0, regT2), target + 3);
}

void JIT::emitSlow_op_loop_if_lesseq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned op1 = currentInstruction[1].u.operand;
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    if (!isOperandConstantImmediateInt(op1) && !isOperandConstantImmediateInt(op2))
        linkSlowCase(iter); // int32 check
    linkSlowCase(iter); // int32 check

    JITStubCall stubCall(this, cti_op_loop_if_lesseq);
    stubCall.addArgument(op1);
    stubCall.addArgument(op2);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 3);
}

void JIT::emit_op_new_object(Instruction* currentInstruction)
{
    JITStubCall(this, cti_op_new_object).call(currentInstruction[1].u.operand);
}

void JIT::emit_op_instanceof(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned value = currentInstruction[2].u.operand;
    unsigned baseVal = currentInstruction[3].u.operand;
    unsigned proto = currentInstruction[4].u.operand;

    // Load the operands (baseVal, proto, and value respectively) into registers.
    // We use regT0 for baseVal since we will be done with this first, and we can then use it for the result.
    emitLoadPayload(proto, regT1);
    emitLoadPayload(baseVal, regT0);
    emitLoadPayload(value, regT2);

    // Check that baseVal & proto are cells.
    emitJumpSlowCaseIfNotJSCell(proto);
    emitJumpSlowCaseIfNotJSCell(baseVal);

    // Check that baseVal is an object, that it 'ImplementsHasInstance' but that it does not 'OverridesHasInstance'.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT0);
    addSlowCase(branch32(NotEqual, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_type)), Imm32(ObjectType))); // FIXME: Maybe remove this test.
    addSlowCase(branchTest32(Zero, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(ImplementsHasInstance))); // FIXME: TOT checks ImplementsDefaultHasInstance.

    // If value is not an Object, return false.
    emitLoadTag(value, regT0);
    Jump valueIsImmediate = branch32(NotEqual, regT0, Imm32(JSValue::CellTag));
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSCell, m_structure)), regT0);
    Jump valueIsNotObject = branch32(NotEqual, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_type)), Imm32(ObjectType)); // FIXME: Maybe remove this test.

    // Check proto is object.
    loadPtr(Address(regT1, OBJECT_OFFSETOF(JSCell, m_structure)), regT0);
    addSlowCase(branch32(NotEqual, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_type)), Imm32(ObjectType)));

    // Optimistically load the result true, and start looping.
    // Initially, regT1 still contains proto and regT2 still contains value.
    // As we loop regT2 will be updated with its prototype, recursively walking the prototype chain.
    move(Imm32(JSValue::TrueTag), regT0);
    Label loop(this);

    // Load the prototype of the object in regT2.  If this is equal to regT1 - WIN!
    // Otherwise, check if we've hit null - if we have then drop out of the loop, if not go again.
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    load32(Address(regT2, OBJECT_OFFSETOF(Structure, m_prototype) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), regT2);
    Jump isInstance = branchPtr(Equal, regT2, regT1);
    branch32(NotEqual, regT2, Imm32(0), loop);

    // We get here either by dropping out of the loop, or if value was not an Object.  Result is false.
    valueIsImmediate.link(this);
    valueIsNotObject.link(this);
    move(Imm32(JSValue::FalseTag), regT0);

    // isInstance jumps right down to here, to skip setting the result to false (it has already set true).
    isInstance.link(this);
    emitStoreBool(dst, regT0);
}

void JIT::emitSlow_op_instanceof(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned value = currentInstruction[2].u.operand;
    unsigned baseVal = currentInstruction[3].u.operand;
    unsigned proto = currentInstruction[4].u.operand;

    linkSlowCaseIfNotJSCell(iter, baseVal);
    linkSlowCaseIfNotJSCell(iter, proto);
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_instanceof);
    stubCall.addArgument(value);
    stubCall.addArgument(baseVal);
    stubCall.addArgument(proto);
    stubCall.call(dst);
}

void JIT::emit_op_new_func(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_func);
    stubCall.addArgument(ImmPtr(m_codeBlock->function(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_get_global_var(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    JSGlobalObject* globalObject = static_cast<JSGlobalObject*>(currentInstruction[2].u.jsCell);
    ASSERT(globalObject->isGlobalObject());
    int index = currentInstruction[3].u.operand;

    loadPtr(&globalObject->d()->registers, regT2);

    emitLoad(index, regT1, regT0, regT2);
    emitStore(dst, regT1, regT0);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_get_global_var), dst, regT1, regT0);
}

void JIT::emit_op_put_global_var(Instruction* currentInstruction)
{
    JSGlobalObject* globalObject = static_cast<JSGlobalObject*>(currentInstruction[1].u.jsCell);
    ASSERT(globalObject->isGlobalObject());
    int index = currentInstruction[2].u.operand;
    int value = currentInstruction[3].u.operand;

    emitLoad(value, regT1, regT0);

    loadPtr(&globalObject->d()->registers, regT2);
    emitStore(index, regT1, regT0, regT2);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_put_global_var), value, regT1, regT0);
}

void JIT::emit_op_get_scoped_var(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int index = currentInstruction[2].u.operand;
    int skip = currentInstruction[3].u.operand + m_codeBlock->needsFullScopeChain();

    emitGetFromCallFrameHeaderPtr(RegisterFile::ScopeChain, regT2);
    while (skip--)
        loadPtr(Address(regT2, OBJECT_OFFSETOF(ScopeChainNode, next)), regT2);

    loadPtr(Address(regT2, OBJECT_OFFSETOF(ScopeChainNode, object)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSVariableObject, d)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSVariableObject::JSVariableObjectData, registers)), regT2);

    emitLoad(index, regT1, regT0, regT2);
    emitStore(dst, regT1, regT0);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_get_scoped_var), dst, regT1, regT0);
}

void JIT::emit_op_put_scoped_var(Instruction* currentInstruction)
{
    int index = currentInstruction[1].u.operand;
    int skip = currentInstruction[2].u.operand + m_codeBlock->needsFullScopeChain();
    int value = currentInstruction[3].u.operand;

    emitLoad(value, regT1, regT0);

    emitGetFromCallFrameHeaderPtr(RegisterFile::ScopeChain, regT2);
    while (skip--)
        loadPtr(Address(regT2, OBJECT_OFFSETOF(ScopeChainNode, next)), regT2);

    loadPtr(Address(regT2, OBJECT_OFFSETOF(ScopeChainNode, object)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSVariableObject, d)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSVariableObject::JSVariableObjectData, registers)), regT2);

    emitStore(index, regT1, regT0, regT2);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_put_scoped_var), value, regT1, regT0);
}

void JIT::emit_op_tear_off_activation(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_tear_off_activation);
    stubCall.addArgument(currentInstruction[1].u.operand);
    stubCall.call();
}

void JIT::emit_op_tear_off_arguments(Instruction*)
{
    JITStubCall(this, cti_op_tear_off_arguments).call();
}

void JIT::emit_op_new_array(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_array);
    stubCall.addArgument(Imm32(currentInstruction[2].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_resolve(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_to_primitive(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src = currentInstruction[2].u.operand;

    emitLoad(src, regT1, regT0);

    Jump isImm = branch32(NotEqual, regT1, Imm32(JSValue::CellTag));
    addSlowCase(branchPtr(NotEqual, Address(regT0), ImmPtr(m_globalData->jsStringVPtr)));
    isImm.link(this);

    if (dst != src)
        emitStore(dst, regT1, regT0);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_to_primitive), dst, regT1, regT0);
}

void JIT::emitSlow_op_to_primitive(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int dst = currentInstruction[1].u.operand;

    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_to_primitive);
    stubCall.addArgument(regT1, regT0);
    stubCall.call(dst);
}

void JIT::emit_op_strcat(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_strcat);
    stubCall.addArgument(Imm32(currentInstruction[2].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_loop_if_true(Instruction* currentInstruction)
{
    unsigned cond = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitTimeoutCheck();

    emitLoad(cond, regT1, regT0);

    Jump isNotInteger = branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag));
    addJump(branch32(NotEqual, regT0, Imm32(0)), target + 2);
    Jump isNotZero = jump();

    isNotInteger.link(this);

    addJump(branch32(Equal, regT1, Imm32(JSValue::TrueTag)), target + 2);
    addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::FalseTag)));

    isNotZero.link(this);
}

void JIT::emitSlow_op_loop_if_true(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned cond = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_jtrue);
    stubCall.addArgument(cond);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 2);
}

void JIT::emit_op_resolve_base(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve_base);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_resolve_skip(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve_skip);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand + m_codeBlock->needsFullScopeChain()));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_resolve_global(Instruction* currentInstruction)
{
    // FIXME: Optimize to use patching instead of so many memory accesses.

    unsigned dst = currentInstruction[1].u.operand;
    void* globalObject = currentInstruction[2].u.jsCell;
    
    unsigned currentIndex = m_globalResolveInfoIndex++;
    void* structureAddress = &(m_codeBlock->globalResolveInfo(currentIndex).structure);
    void* offsetAddr = &(m_codeBlock->globalResolveInfo(currentIndex).offset);

    // Verify structure.
    move(ImmPtr(globalObject), regT0);
    loadPtr(structureAddress, regT1);
    addSlowCase(branchPtr(NotEqual, regT1, Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure))));

    // Load property.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSGlobalObject, m_externalStorage)), regT2);
    load32(offsetAddr, regT3);
    load32(BaseIndex(regT2, regT3, TimesEight), regT0); // payload
    load32(BaseIndex(regT2, regT3, TimesEight, 4), regT1); // tag
    emitStore(dst, regT1, regT0);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_resolve_global), dst, regT1, regT0);
}

void JIT::emitSlow_op_resolve_global(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned dst = currentInstruction[1].u.operand;
    void* globalObject = currentInstruction[2].u.jsCell;
    Identifier* ident = &m_codeBlock->identifier(currentInstruction[3].u.operand);

    unsigned currentIndex = m_globalResolveInfoIndex++;

    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_resolve_global);
    stubCall.addArgument(ImmPtr(globalObject));
    stubCall.addArgument(ImmPtr(ident));
    stubCall.addArgument(Imm32(currentIndex));
    stubCall.call(dst);
}

void JIT::emit_op_not(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src = currentInstruction[2].u.operand;

    emitLoadTag(src, regT0);

    xor32(Imm32(JSValue::FalseTag), regT0);
    addSlowCase(branchTest32(NonZero, regT0, Imm32(~1)));
    xor32(Imm32(JSValue::TrueTag), regT0);

    emitStoreBool(dst, regT0, (dst == src));
}

void JIT::emitSlow_op_not(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src = currentInstruction[2].u.operand;

    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_not);
    stubCall.addArgument(src);
    stubCall.call(dst);
}

void JIT::emit_op_jfalse(Instruction* currentInstruction)
{
    unsigned cond = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitLoad(cond, regT1, regT0);

    Jump isTrue = branch32(Equal, regT1, Imm32(JSValue::TrueTag));
    addJump(branch32(Equal, regT1, Imm32(JSValue::FalseTag)), target + 2);

    Jump isNotInteger = branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag));
    Jump isTrue2 = branch32(NotEqual, regT0, Imm32(0));
    addJump(jump(), target + 2);

    isNotInteger.link(this);

    addSlowCase(branch32(Above, regT1, Imm32(JSValue::LowestTag)));

    zeroDouble(fpRegT0);
    emitLoadDouble(cond, fpRegT1);
    addJump(branchDouble(DoubleEqual, fpRegT0, fpRegT1), target + 2);
    
    isTrue.link(this);
    isTrue2.link(this);
}

void JIT::emitSlow_op_jfalse(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned cond = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_jtrue);
    stubCall.addArgument(cond);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(Zero, regT0), target + 2); // Inverted.
}

void JIT::emit_op_jtrue(Instruction* currentInstruction)
{
    unsigned cond = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitLoad(cond, regT1, regT0);

    Jump isFalse = branch32(Equal, regT1, Imm32(JSValue::FalseTag));
    addJump(branch32(Equal, regT1, Imm32(JSValue::TrueTag)), target + 2);

    Jump isNotInteger = branch32(NotEqual, regT1, Imm32(JSValue::Int32Tag));
    Jump isFalse2 = branch32(Equal, regT0, Imm32(0));
    addJump(jump(), target + 2);

    isNotInteger.link(this);

    addSlowCase(branch32(Above, regT1, Imm32(JSValue::LowestTag)));

    zeroDouble(fpRegT0);
    emitLoadDouble(cond, fpRegT1);
    addJump(branchDouble(DoubleNotEqual, fpRegT0, fpRegT1), target + 2);
    
    isFalse.link(this);
    isFalse2.link(this);
}

void JIT::emitSlow_op_jtrue(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned cond = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_jtrue);
    stubCall.addArgument(cond);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 2);
}

void JIT::emit_op_jeq_null(Instruction* currentInstruction)
{
    unsigned src = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitLoad(src, regT1, regT0);

    Jump isImmediate = branch32(NotEqual, regT1, Imm32(JSValue::CellTag));

    // First, handle JSCell cases - check MasqueradesAsUndefined bit on the structure.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    addJump(branchTest32(NonZero, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined)), target + 2);

    Jump wasNotImmediate = jump();

    // Now handle the immediate cases - undefined & null
    isImmediate.link(this);

    set32(Equal, regT1, Imm32(JSValue::NullTag), regT2);
    set32(Equal, regT1, Imm32(JSValue::UndefinedTag), regT1);
    or32(regT2, regT1);

    addJump(branchTest32(NonZero, regT1), target + 2);

    wasNotImmediate.link(this);
}

void JIT::emit_op_jneq_null(Instruction* currentInstruction)
{
    unsigned src = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitLoad(src, regT1, regT0);

    Jump isImmediate = branch32(NotEqual, regT1, Imm32(JSValue::CellTag));

    // First, handle JSCell cases - check MasqueradesAsUndefined bit on the structure.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    addJump(branchTest32(Zero, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined)), target + 2);

    Jump wasNotImmediate = jump();

    // Now handle the immediate cases - undefined & null
    isImmediate.link(this);

    set32(Equal, regT1, Imm32(JSValue::NullTag), regT2);
    set32(Equal, regT1, Imm32(JSValue::UndefinedTag), regT1);
    or32(regT2, regT1);

    addJump(branchTest32(Zero, regT1), target + 2);

    wasNotImmediate.link(this);
}

void JIT::emit_op_jneq_ptr(Instruction* currentInstruction)
{
    unsigned src = currentInstruction[1].u.operand;
    JSCell* ptr = currentInstruction[2].u.jsCell;
    unsigned target = currentInstruction[3].u.operand;

    emitLoad(src, regT1, regT0);
    addJump(branch32(NotEqual, regT1, Imm32(JSValue::CellTag)), target + 3);
    addJump(branchPtr(NotEqual, regT0, ImmPtr(ptr)), target + 3);
}

void JIT::emit_op_jsr(Instruction* currentInstruction)
{
    int retAddrDst = currentInstruction[1].u.operand;
    int target = currentInstruction[2].u.operand;
    DataLabelPtr storeLocation = storePtrWithPatch(ImmPtr(0), Address(callFrameRegister, sizeof(Register) * retAddrDst));
    addJump(jump(), target + 2);
    m_jsrSites.append(JSRInfo(storeLocation, label()));
}

void JIT::emit_op_sret(Instruction* currentInstruction)
{
    jump(Address(callFrameRegister, sizeof(Register) * currentInstruction[1].u.operand));
}

void JIT::emit_op_eq(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;
    unsigned src2 = currentInstruction[3].u.operand;
    
    emitLoad2(src1, regT1, regT0, src2, regT3, regT2);
    addSlowCase(branch32(NotEqual, regT1, regT3));
    addSlowCase(branch32(Equal, regT1, Imm32(JSValue::CellTag)));
    addSlowCase(branch32(Below, regT1, Imm32(JSValue::LowestTag)));

    set8(Equal, regT0, regT2, regT0);
    or32(Imm32(JSValue::FalseTag), regT0);

    emitStoreBool(dst, regT0);
}

void JIT::emitSlow_op_eq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned op1 = currentInstruction[2].u.operand;
    unsigned op2 = currentInstruction[3].u.operand;
    
    JumpList storeResult;
    JumpList genericCase;
    
    genericCase.append(getSlowCase(iter)); // tags not equal

    linkSlowCase(iter); // tags equal and JSCell
    genericCase.append(branchPtr(NotEqual, Address(regT0), ImmPtr(m_globalData->jsStringVPtr)));
    genericCase.append(branchPtr(NotEqual, Address(regT2), ImmPtr(m_globalData->jsStringVPtr)));

    // String case.
    JITStubCall stubCallEqStrings(this, cti_op_eq_strings);
    stubCallEqStrings.addArgument(regT0);
    stubCallEqStrings.addArgument(regT2);
    stubCallEqStrings.call();
    storeResult.append(jump());

    // Generic case.
    genericCase.append(getSlowCase(iter)); // doubles
    genericCase.link(this);
    JITStubCall stubCallEq(this, cti_op_eq);
    stubCallEq.addArgument(op1);
    stubCallEq.addArgument(op2);
    stubCallEq.call(regT0);

    storeResult.link(this);
    or32(Imm32(JSValue::FalseTag), regT0);
    emitStoreBool(dst, regT0);
}

void JIT::emit_op_neq(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;
    unsigned src2 = currentInstruction[3].u.operand;
    
    emitLoad2(src1, regT1, regT0, src2, regT3, regT2);
    addSlowCase(branch32(NotEqual, regT1, regT3));
    addSlowCase(branch32(Equal, regT1, Imm32(JSValue::CellTag)));
    addSlowCase(branch32(Below, regT1, Imm32(JSValue::LowestTag)));

    set8(NotEqual, regT0, regT2, regT0);
    or32(Imm32(JSValue::FalseTag), regT0);

    emitStoreBool(dst, regT0);
}

void JIT::emitSlow_op_neq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned dst = currentInstruction[1].u.operand;
    
    JumpList storeResult;
    JumpList genericCase;
    
    genericCase.append(getSlowCase(iter)); // tags not equal

    linkSlowCase(iter); // tags equal and JSCell
    genericCase.append(branchPtr(NotEqual, Address(regT0), ImmPtr(m_globalData->jsStringVPtr)));
    genericCase.append(branchPtr(NotEqual, Address(regT2), ImmPtr(m_globalData->jsStringVPtr)));

    // String case.
    JITStubCall stubCallEqStrings(this, cti_op_eq_strings);
    stubCallEqStrings.addArgument(regT0);
    stubCallEqStrings.addArgument(regT2);
    stubCallEqStrings.call(regT0);
    storeResult.append(jump());

    // Generic case.
    genericCase.append(getSlowCase(iter)); // doubles
    genericCase.link(this);
    JITStubCall stubCallEq(this, cti_op_eq);
    stubCallEq.addArgument(regT1, regT0);
    stubCallEq.addArgument(regT3, regT2);
    stubCallEq.call(regT0);

    storeResult.link(this);
    xor32(Imm32(0x1), regT0);
    or32(Imm32(JSValue::FalseTag), regT0);
    emitStoreBool(dst, regT0);
}

void JIT::compileOpStrictEq(Instruction* currentInstruction, CompileOpStrictEqType type)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;
    unsigned src2 = currentInstruction[3].u.operand;

    emitLoadTag(src1, regT0);
    emitLoadTag(src2, regT1);

    // Jump to a slow case if either operand is double, or if both operands are
    // cells and/or Int32s.
    move(regT0, regT2);
    and32(regT1, regT2);
    addSlowCase(branch32(Below, regT2, Imm32(JSValue::LowestTag)));
    addSlowCase(branch32(AboveOrEqual, regT2, Imm32(JSValue::CellTag)));

    if (type == OpStrictEq)
        set8(Equal, regT0, regT1, regT0);
    else
        set8(NotEqual, regT0, regT1, regT0);

    or32(Imm32(JSValue::FalseTag), regT0);

    emitStoreBool(dst, regT0);
}

void JIT::emit_op_stricteq(Instruction* currentInstruction)
{
    compileOpStrictEq(currentInstruction, OpStrictEq);
}

void JIT::emitSlow_op_stricteq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;
    unsigned src2 = currentInstruction[3].u.operand;

    linkSlowCase(iter);
    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_stricteq);
    stubCall.addArgument(src1);
    stubCall.addArgument(src2);
    stubCall.call(dst);
}

void JIT::emit_op_nstricteq(Instruction* currentInstruction)
{
    compileOpStrictEq(currentInstruction, OpNStrictEq);
}

void JIT::emitSlow_op_nstricteq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;
    unsigned src2 = currentInstruction[3].u.operand;

    linkSlowCase(iter);
    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_nstricteq);
    stubCall.addArgument(src1);
    stubCall.addArgument(src2);
    stubCall.call(dst);
}

void JIT::emit_op_eq_null(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src = currentInstruction[2].u.operand;

    emitLoad(src, regT1, regT0);
    Jump isImmediate = branch32(NotEqual, regT1, Imm32(JSValue::CellTag));

    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT1);
    setTest8(NonZero, Address(regT1, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined), regT1);

    Jump wasNotImmediate = jump();

    isImmediate.link(this);

    set8(Equal, regT1, Imm32(JSValue::NullTag), regT2);
    set8(Equal, regT1, Imm32(JSValue::UndefinedTag), regT1);
    or32(regT2, regT1);

    wasNotImmediate.link(this);

    or32(Imm32(JSValue::FalseTag), regT1);

    emitStoreBool(dst, regT1);
}

void JIT::emit_op_neq_null(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src = currentInstruction[2].u.operand;

    emitLoad(src, regT1, regT0);
    Jump isImmediate = branch32(NotEqual, regT1, Imm32(JSValue::CellTag));

    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT1);
    setTest8(Zero, Address(regT1, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined), regT1);

    Jump wasNotImmediate = jump();

    isImmediate.link(this);

    set8(NotEqual, regT1, Imm32(JSValue::NullTag), regT2);
    set8(NotEqual, regT1, Imm32(JSValue::UndefinedTag), regT1);
    and32(regT2, regT1);

    wasNotImmediate.link(this);

    or32(Imm32(JSValue::FalseTag), regT1);

    emitStoreBool(dst, regT1);
}

void JIT::emit_op_resolve_with_base(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve_with_base);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[3].u.operand)));
    stubCall.addArgument(Imm32(currentInstruction[1].u.operand));
    stubCall.call(currentInstruction[2].u.operand);
}

void JIT::emit_op_new_func_exp(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_func_exp);
    stubCall.addArgument(ImmPtr(m_codeBlock->functionExpression(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_new_regexp(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_regexp);
    stubCall.addArgument(ImmPtr(m_codeBlock->regexp(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_throw(Instruction* currentInstruction)
{
    unsigned exception = currentInstruction[1].u.operand;
    JITStubCall stubCall(this, cti_op_throw);
    stubCall.addArgument(exception);
    stubCall.call();

#ifndef NDEBUG
    // cti_op_throw always changes it's return address,
    // this point in the code should never be reached.
    breakpoint();
#endif
}

void JIT::emit_op_next_pname(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int iter = currentInstruction[2].u.operand;
    int target = currentInstruction[3].u.operand;

    load32(Address(callFrameRegister, (iter * sizeof(Register))), regT0);

    JITStubCall stubCall(this, cti_op_next_pname);
    stubCall.addArgument(regT0);
    stubCall.call();

    Jump endOfIter = branchTestPtr(Zero, regT0);
    emitStore(dst, regT1, regT0);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_next_pname), dst, regT1, regT0);
    addJump(jump(), target + 3);
    endOfIter.link(this);
}

void JIT::emit_op_push_scope(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_push_scope);
    stubCall.addArgument(currentInstruction[1].u.operand);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_pop_scope(Instruction*)
{
    JITStubCall(this, cti_op_pop_scope).call();
}

void JIT::emit_op_to_jsnumber(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src = currentInstruction[2].u.operand;

    emitLoad(src, regT1, regT0);

    Jump isInt32 = branch32(Equal, regT1, Imm32(JSValue::Int32Tag));
    addSlowCase(branch32(AboveOrEqual, regT1, Imm32(JSValue::DeletedValueTag)));
    isInt32.link(this);

    if (src != dst)
        emitStore(dst, regT1, regT0);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_to_jsnumber), dst, regT1, regT0);
}

void JIT::emitSlow_op_to_jsnumber(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int dst = currentInstruction[1].u.operand;

    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_to_jsnumber);
    stubCall.addArgument(regT1, regT0);
    stubCall.call(dst);
}

void JIT::emit_op_push_new_scope(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_push_new_scope);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.addArgument(currentInstruction[3].u.operand);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_catch(Instruction* currentInstruction)
{
    unsigned exception = currentInstruction[1].u.operand;

    // This opcode only executes after a return from cti_op_throw.

    // cti_op_throw may have taken us to a call frame further up the stack; reload
    // the call frame pointer to adjust.
    peek(callFrameRegister, OBJECT_OFFSETOF(struct JITStackFrame, callFrame) / sizeof (void*));

    // Now store the exception returned by cti_op_throw.
    emitStore(exception, regT1, regT0);
    map(m_bytecodeIndex + OPCODE_LENGTH(op_catch), exception, regT1, regT0);
}

void JIT::emit_op_jmp_scopes(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_jmp_scopes);
    stubCall.addArgument(Imm32(currentInstruction[1].u.operand));
    stubCall.call();
    addJump(jump(), currentInstruction[2].u.operand + 2);
}

void JIT::emit_op_switch_imm(Instruction* currentInstruction)
{
    unsigned tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    SimpleJumpTable* jumpTable = &m_codeBlock->immediateSwitchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeIndex, defaultOffset, SwitchRecord::Immediate));
    jumpTable->ctiOffsets.grow(jumpTable->branchOffsets.size());

    JITStubCall stubCall(this, cti_op_switch_imm);
    stubCall.addArgument(scrutinee);
    stubCall.addArgument(Imm32(tableIndex));
    stubCall.call();
    jump(regT0);
}

void JIT::emit_op_switch_char(Instruction* currentInstruction)
{
    unsigned tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    SimpleJumpTable* jumpTable = &m_codeBlock->characterSwitchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeIndex, defaultOffset, SwitchRecord::Character));
    jumpTable->ctiOffsets.grow(jumpTable->branchOffsets.size());

    JITStubCall stubCall(this, cti_op_switch_char);
    stubCall.addArgument(scrutinee);
    stubCall.addArgument(Imm32(tableIndex));
    stubCall.call();
    jump(regT0);
}

void JIT::emit_op_switch_string(Instruction* currentInstruction)
{
    unsigned tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    StringJumpTable* jumpTable = &m_codeBlock->stringSwitchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeIndex, defaultOffset));

    JITStubCall stubCall(this, cti_op_switch_string);
    stubCall.addArgument(scrutinee);
    stubCall.addArgument(Imm32(tableIndex));
    stubCall.call();
    jump(regT0);
}

void JIT::emit_op_new_error(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned type = currentInstruction[2].u.operand;
    unsigned message = currentInstruction[3].u.operand;

    JITStubCall stubCall(this, cti_op_new_error);
    stubCall.addArgument(Imm32(type));
    stubCall.addArgument(m_codeBlock->getConstant(message));
    stubCall.addArgument(Imm32(m_bytecodeIndex));
    stubCall.call(dst);
}

void JIT::emit_op_debug(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_debug);
    stubCall.addArgument(Imm32(currentInstruction[1].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[2].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand));
    stubCall.call();
}


void JIT::emit_op_enter(Instruction*)
{
    // Even though JIT code doesn't use them, we initialize our constant
    // registers to zap stale pointers, to avoid unnecessarily prolonging
    // object lifetime and increasing GC pressure.
    for (int i = 0; i < m_codeBlock->m_numVars; ++i)
        emitStore(i, jsUndefined());
}

void JIT::emit_op_enter_with_activation(Instruction* currentInstruction)
{
    emit_op_enter(currentInstruction);

    JITStubCall(this, cti_op_push_activation).call(currentInstruction[1].u.operand);
}

void JIT::emit_op_create_arguments(Instruction*)
{
    Jump argsNotCell = branch32(NotEqual, tagFor(RegisterFile::ArgumentsRegister, callFrameRegister), Imm32(JSValue::CellTag));
    Jump argsNotNull = branchTestPtr(NonZero, payloadFor(RegisterFile::ArgumentsRegister, callFrameRegister));

    // If we get here the arguments pointer is a null cell - i.e. arguments need lazy creation.
    if (m_codeBlock->m_numParameters == 1)
        JITStubCall(this, cti_op_create_arguments_no_params).call();
    else
        JITStubCall(this, cti_op_create_arguments).call();

    argsNotCell.link(this);
    argsNotNull.link(this);
}
    
void JIT::emit_op_init_arguments(Instruction*)
{
    emitStore(RegisterFile::ArgumentsRegister, JSValue(), callFrameRegister);
}

void JIT::emit_op_convert_this(Instruction* currentInstruction)
{
    unsigned thisRegister = currentInstruction[1].u.operand;
    
    emitLoad(thisRegister, regT1, regT0);

    addSlowCase(branch32(NotEqual, regT1, Imm32(JSValue::CellTag)));

    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    addSlowCase(branchTest32(NonZero, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(NeedsThisConversion)));

    map(m_bytecodeIndex + OPCODE_LENGTH(op_convert_this), thisRegister, regT1, regT0);
}

void JIT::emitSlow_op_convert_this(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned thisRegister = currentInstruction[1].u.operand;

    linkSlowCase(iter);
    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_convert_this);
    stubCall.addArgument(regT1, regT0);
    stubCall.call(thisRegister);
}

void JIT::emit_op_profile_will_call(Instruction* currentInstruction)
{
    peek(regT2, OBJECT_OFFSETOF(JITStackFrame, enabledProfilerReference) / sizeof (void*));
    Jump noProfiler = branchTestPtr(Zero, Address(regT2));

    JITStubCall stubCall(this, cti_op_profile_will_call);
    stubCall.addArgument(currentInstruction[1].u.operand);
    stubCall.call();
    noProfiler.link(this);
}

void JIT::emit_op_profile_did_call(Instruction* currentInstruction)
{
    peek(regT2, OBJECT_OFFSETOF(JITStackFrame, enabledProfilerReference) / sizeof (void*));
    Jump noProfiler = branchTestPtr(Zero, Address(regT2));

    JITStubCall stubCall(this, cti_op_profile_did_call);
    stubCall.addArgument(currentInstruction[1].u.operand);
    stubCall.call();
    noProfiler.link(this);
}

#else // USE(JSVALUE32_64)

#define RECORD_JUMP_TARGET(targetOffset) \
   do { m_labels[m_bytecodeIndex + (targetOffset)].used(); } while (false)

void JIT::privateCompileCTIMachineTrampolines(RefPtr<ExecutablePool>* executablePool, JSGlobalData* globalData, CodePtr* ctiStringLengthTrampoline, CodePtr* ctiVirtualCallPreLink, CodePtr* ctiVirtualCallLink, CodePtr* ctiVirtualCall, CodePtr* ctiNativeCallThunk)
{
#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    // (2) The second function provides fast property access for string length
    Label stringLengthBegin = align();

    // Check eax is a string
    Jump string_failureCases1 = emitJumpIfNotJSCell(regT0);
    Jump string_failureCases2 = branchPtr(NotEqual, Address(regT0), ImmPtr(m_globalData->jsStringVPtr));

    // Checks out okay! - get the length from the Ustring.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSString, m_value) + OBJECT_OFFSETOF(UString, m_rep)), regT0);
    load32(Address(regT0, OBJECT_OFFSETOF(UString::Rep, len)), regT0);

    Jump string_failureCases3 = branch32(Above, regT0, Imm32(JSImmediate::maxImmediateInt));

    // regT0 contains a 64 bit value (is positive, is zero extended) so we don't need sign extend here.
    emitFastArithIntToImmNoCheck(regT0, regT0);
    
    ret();
#endif

    // (3) Trampolines for the slow cases of op_call / op_call_eval / op_construct.
    COMPILE_ASSERT(sizeof(CodeType) == 4, CodeTypeEnumMustBe32Bit);

    Label virtualCallPreLinkBegin = align();

    // Load the callee CodeBlock* into eax
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSFunction, m_body)), regT3);
    loadPtr(Address(regT3, OBJECT_OFFSETOF(FunctionBodyNode, m_code)), regT0);
    Jump hasCodeBlock1 = branchTestPtr(NonZero, regT0);
    preserveReturnAddressAfterCall(regT3);
    restoreArgumentReference();
    Call callJSFunction1 = call();
    emitGetJITStubArg(1, regT2);
    emitGetJITStubArg(3, regT1);
    restoreReturnAddressBeforeReturn(regT3);
    hasCodeBlock1.link(this);

    Jump isNativeFunc1 = branch32(Equal, Address(regT0, OBJECT_OFFSETOF(CodeBlock, m_codeType)), Imm32(NativeCode));

    // Check argCount matches callee arity.
    Jump arityCheckOkay1 = branch32(Equal, Address(regT0, OBJECT_OFFSETOF(CodeBlock, m_numParameters)), regT1);
    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 2);
    emitPutJITStubArg(regT0, 4);
    restoreArgumentReference();
    Call callArityCheck1 = call();
    move(regT1, callFrameRegister);
    emitGetJITStubArg(1, regT2);
    emitGetJITStubArg(3, regT1);
    restoreReturnAddressBeforeReturn(regT3);
    arityCheckOkay1.link(this);
    isNativeFunc1.link(this);
    
    compileOpCallInitializeCallFrame();

    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 2);
    restoreArgumentReference();
    Call callDontLazyLinkCall = call();
    emitGetJITStubArg(1, regT2);
    restoreReturnAddressBeforeReturn(regT3);

    jump(regT0);

    Label virtualCallLinkBegin = align();

    // Load the callee CodeBlock* into eax
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSFunction, m_body)), regT3);
    loadPtr(Address(regT3, OBJECT_OFFSETOF(FunctionBodyNode, m_code)), regT0);
    Jump hasCodeBlock2 = branchTestPtr(NonZero, regT0);
    preserveReturnAddressAfterCall(regT3);
    restoreArgumentReference();
    Call callJSFunction2 = call();
    emitGetJITStubArg(1, regT2);
    emitGetJITStubArg(3, regT1);
    restoreReturnAddressBeforeReturn(regT3);
    hasCodeBlock2.link(this);

    Jump isNativeFunc2 = branch32(Equal, Address(regT0, OBJECT_OFFSETOF(CodeBlock, m_codeType)), Imm32(NativeCode));

    // Check argCount matches callee arity.
    Jump arityCheckOkay2 = branch32(Equal, Address(regT0, OBJECT_OFFSETOF(CodeBlock, m_numParameters)), regT1);
    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 2);
    emitPutJITStubArg(regT0, 4);
    restoreArgumentReference();
    Call callArityCheck2 = call();
    move(regT1, callFrameRegister);
    emitGetJITStubArg(1, regT2);
    emitGetJITStubArg(3, regT1);
    restoreReturnAddressBeforeReturn(regT3);
    arityCheckOkay2.link(this);
    isNativeFunc2.link(this);

    compileOpCallInitializeCallFrame();

    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 2);
    restoreArgumentReference();
    Call callLazyLinkCall = call();
    restoreReturnAddressBeforeReturn(regT3);

    jump(regT0);

    Label virtualCallBegin = align();

    // Load the callee CodeBlock* into eax
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSFunction, m_body)), regT3);
    loadPtr(Address(regT3, OBJECT_OFFSETOF(FunctionBodyNode, m_code)), regT0);
    Jump hasCodeBlock3 = branchTestPtr(NonZero, regT0);
    preserveReturnAddressAfterCall(regT3);
    restoreArgumentReference();
    Call callJSFunction3 = call();
    emitGetJITStubArg(1, regT2);
    emitGetJITStubArg(3, regT1);
    restoreReturnAddressBeforeReturn(regT3);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSFunction, m_body)), regT3); // reload the function body nody, so we can reload the code pointer.
    hasCodeBlock3.link(this);
    
    Jump isNativeFunc3 = branch32(Equal, Address(regT0, OBJECT_OFFSETOF(CodeBlock, m_codeType)), Imm32(NativeCode));

    // Check argCount matches callee arity.
    Jump arityCheckOkay3 = branch32(Equal, Address(regT0, OBJECT_OFFSETOF(CodeBlock, m_numParameters)), regT1);
    preserveReturnAddressAfterCall(regT3);
    emitPutJITStubArg(regT3, 2);
    emitPutJITStubArg(regT0, 4);
    restoreArgumentReference();
    Call callArityCheck3 = call();
    move(regT1, callFrameRegister);
    emitGetJITStubArg(1, regT2);
    emitGetJITStubArg(3, regT1);
    restoreReturnAddressBeforeReturn(regT3);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSFunction, m_body)), regT3); // reload the function body nody, so we can reload the code pointer.
    arityCheckOkay3.link(this);
    isNativeFunc3.link(this);

    // load ctiCode from the new codeBlock.
    loadPtr(Address(regT3, OBJECT_OFFSETOF(FunctionBodyNode, m_jitCode)), regT0);
    
    compileOpCallInitializeCallFrame();
    jump(regT0);

    
    Label nativeCallThunk = align();
    preserveReturnAddressAfterCall(regT0);
    emitPutToCallFrameHeader(regT0, RegisterFile::ReturnPC); // Push return address

    // Load caller frame's scope chain into this callframe so that whatever we call can
    // get to its global data.
    emitGetFromCallFrameHeaderPtr(RegisterFile::CallerFrame, regT1);
    emitGetFromCallFrameHeaderPtr(RegisterFile::ScopeChain, regT1, regT1);
    emitPutToCallFrameHeader(regT1, RegisterFile::ScopeChain);
    

#if PLATFORM(X86_64)
    emitGetFromCallFrameHeader32(RegisterFile::ArgumentCount, X86::ecx);

    // Allocate stack space for our arglist
    subPtr(Imm32(sizeof(ArgList)), stackPointerRegister);
    COMPILE_ASSERT((sizeof(ArgList) & 0xf) == 0, ArgList_should_by_16byte_aligned);
    
    // Set up arguments
    subPtr(Imm32(1), X86::ecx); // Don't include 'this' in argcount

    // Push argcount
    storePtr(X86::ecx, Address(stackPointerRegister, OBJECT_OFFSETOF(ArgList, m_argCount)));

    // Calculate the start of the callframe header, and store in edx
    addPtr(Imm32(-RegisterFile::CallFrameHeaderSize * (int32_t)sizeof(Register)), callFrameRegister, X86::edx);
    
    // Calculate start of arguments as callframe header - sizeof(Register) * argcount (ecx)
    mul32(Imm32(sizeof(Register)), X86::ecx, X86::ecx);
    subPtr(X86::ecx, X86::edx);

    // push pointer to arguments
    storePtr(X86::edx, Address(stackPointerRegister, OBJECT_OFFSETOF(ArgList, m_args)));
    
    // ArgList is passed by reference so is stackPointerRegister
    move(stackPointerRegister, X86::ecx);
    
    // edx currently points to the first argument, edx-sizeof(Register) points to 'this'
    loadPtr(Address(X86::edx, -(int32_t)sizeof(Register)), X86::edx);
    
    emitGetFromCallFrameHeaderPtr(RegisterFile::Callee, X86::esi);

    move(callFrameRegister, X86::edi); 

    call(Address(X86::esi, OBJECT_OFFSETOF(JSFunction, m_data)));
    
    addPtr(Imm32(sizeof(ArgList)), stackPointerRegister);
#elif PLATFORM(X86)
    emitGetFromCallFrameHeader32(RegisterFile::ArgumentCount, regT0);

    /* We have two structs that we use to describe the stackframe we set up for our
     * call to native code.  NativeCallFrameStructure describes the how we set up the stack
     * in advance of the call.  NativeFunctionCalleeSignature describes the callframe
     * as the native code expects it.  We do this as we are using the fastcall calling
     * convention which results in the callee popping its arguments off the stack, but
     * not the rest of the callframe so we need a nice way to ensure we increment the
     * stack pointer by the right amount after the call.
     */
#if COMPILER(MSVC) || PLATFORM(LINUX)
    struct NativeCallFrameStructure {
      //  CallFrame* callFrame; // passed in EDX
        JSObject* callee;
        JSValue thisValue;
        ArgList* argPointer;
        ArgList args;
        JSValue result;
    };
    struct NativeFunctionCalleeSignature {
        JSObject* callee;
        JSValue thisValue;
        ArgList* argPointer;
    };
#else
    struct NativeCallFrameStructure {
      //  CallFrame* callFrame; // passed in ECX
      //  JSObject* callee; // passed in EDX
        JSValue thisValue;
        ArgList* argPointer;
        ArgList args;
    };
    struct NativeFunctionCalleeSignature {
        JSValue thisValue;
        ArgList* argPointer;
    };
#endif
    const int NativeCallFrameSize = (sizeof(NativeCallFrameStructure) + 15) & ~15;
    // Allocate system stack frame
    subPtr(Imm32(NativeCallFrameSize), stackPointerRegister);

    // Set up arguments
    subPtr(Imm32(1), regT0); // Don't include 'this' in argcount

    // push argcount
    storePtr(regT0, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, args) + OBJECT_OFFSETOF(ArgList, m_argCount)));
    
    // Calculate the start of the callframe header, and store in regT1
    addPtr(Imm32(-RegisterFile::CallFrameHeaderSize * (int)sizeof(Register)), callFrameRegister, regT1);
    
    // Calculate start of arguments as callframe header - sizeof(Register) * argcount (regT0)
    mul32(Imm32(sizeof(Register)), regT0, regT0);
    subPtr(regT0, regT1);
    storePtr(regT1, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, args) + OBJECT_OFFSETOF(ArgList, m_args)));

    // ArgList is passed by reference so is stackPointerRegister + 4 * sizeof(Register)
    addPtr(Imm32(OBJECT_OFFSETOF(NativeCallFrameStructure, args)), stackPointerRegister, regT0);
    storePtr(regT0, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, argPointer)));

    // regT1 currently points to the first argument, regT1 - sizeof(Register) points to 'this'
    loadPtr(Address(regT1, -(int)sizeof(Register)), regT1);
    storePtr(regT1, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, thisValue)));

#if COMPILER(MSVC) || PLATFORM(LINUX)
    // ArgList is passed by reference so is stackPointerRegister + 4 * sizeof(Register)
    addPtr(Imm32(OBJECT_OFFSETOF(NativeCallFrameStructure, result)), stackPointerRegister, X86::ecx);

    // Plant callee
    emitGetFromCallFrameHeaderPtr(RegisterFile::Callee, X86::eax);
    storePtr(X86::eax, Address(stackPointerRegister, OBJECT_OFFSETOF(NativeCallFrameStructure, callee)));

    // Plant callframe
    move(callFrameRegister, X86::edx);

    call(Address(X86::eax, OBJECT_OFFSETOF(JSFunction, m_data)));

    // JSValue is a non-POD type
    loadPtr(Address(X86::eax), X86::eax);
#else
    // Plant callee
    emitGetFromCallFrameHeaderPtr(RegisterFile::Callee, X86::edx);

    // Plant callframe
    move(callFrameRegister, X86::ecx);
    call(Address(X86::edx, OBJECT_OFFSETOF(JSFunction, m_data)));
#endif

    // We've put a few temporaries on the stack in addition to the actual arguments
    // so pull them off now
    addPtr(Imm32(NativeCallFrameSize - sizeof(NativeFunctionCalleeSignature)), stackPointerRegister);

#elif ENABLE(JIT_OPTIMIZE_NATIVE_CALL)
#error "JIT_OPTIMIZE_NATIVE_CALL not yet supported on this platform."
#else
    breakpoint();
#endif

    // Check for an exception
    loadPtr(&(globalData->exception), regT2);
    Jump exceptionHandler = branchTestPtr(NonZero, regT2);

    // Grab the return address.
    emitGetFromCallFrameHeaderPtr(RegisterFile::ReturnPC, regT1);
    
    // Restore our caller's "r".
    emitGetFromCallFrameHeaderPtr(RegisterFile::CallerFrame, callFrameRegister);
    
    // Return.
    restoreReturnAddressBeforeReturn(regT1);
    ret();

    // Handle an exception
    exceptionHandler.link(this);
    // Grab the return address.
    emitGetFromCallFrameHeaderPtr(RegisterFile::ReturnPC, regT1);
    move(ImmPtr(&globalData->exceptionLocation), regT2);
    storePtr(regT1, regT2);
    move(ImmPtr(reinterpret_cast<void*>(ctiVMThrowTrampoline)), regT2);
    emitGetFromCallFrameHeaderPtr(RegisterFile::CallerFrame, callFrameRegister);
    poke(callFrameRegister, OBJECT_OFFSETOF(struct JITStackFrame, callFrame) / sizeof (void*));
    restoreReturnAddressBeforeReturn(regT2);
    ret();
    

#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    Call string_failureCases1Call = makeTailRecursiveCall(string_failureCases1);
    Call string_failureCases2Call = makeTailRecursiveCall(string_failureCases2);
    Call string_failureCases3Call = makeTailRecursiveCall(string_failureCases3);
#endif

    // All trampolines constructed! copy the code, link up calls, and set the pointers on the Machine object.
    LinkBuffer patchBuffer(this, m_globalData->executableAllocator.poolForSize(m_assembler.size()));

#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    patchBuffer.link(string_failureCases1Call, FunctionPtr(cti_op_get_by_id_string_fail));
    patchBuffer.link(string_failureCases2Call, FunctionPtr(cti_op_get_by_id_string_fail));
    patchBuffer.link(string_failureCases3Call, FunctionPtr(cti_op_get_by_id_string_fail));
#endif
    patchBuffer.link(callArityCheck1, FunctionPtr(cti_op_call_arityCheck));
    patchBuffer.link(callArityCheck2, FunctionPtr(cti_op_call_arityCheck));
    patchBuffer.link(callArityCheck3, FunctionPtr(cti_op_call_arityCheck));
    patchBuffer.link(callJSFunction1, FunctionPtr(cti_op_call_JSFunction));
    patchBuffer.link(callJSFunction2, FunctionPtr(cti_op_call_JSFunction));
    patchBuffer.link(callJSFunction3, FunctionPtr(cti_op_call_JSFunction));
    patchBuffer.link(callDontLazyLinkCall, FunctionPtr(cti_vm_dontLazyLinkCall));
    patchBuffer.link(callLazyLinkCall, FunctionPtr(cti_vm_lazyLinkCall));

    CodeRef finalCode = patchBuffer.finalizeCode();
    *executablePool = finalCode.m_executablePool;

    *ctiVirtualCallPreLink = trampolineAt(finalCode, virtualCallPreLinkBegin);
    *ctiVirtualCallLink = trampolineAt(finalCode, virtualCallLinkBegin);
    *ctiVirtualCall = trampolineAt(finalCode, virtualCallBegin);
    *ctiNativeCallThunk = trampolineAt(finalCode, nativeCallThunk);
#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    *ctiStringLengthTrampoline = trampolineAt(finalCode, stringLengthBegin);
#else
    UNUSED_PARAM(ctiStringLengthTrampoline);
#endif
}

void JIT::emit_op_mov(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src = currentInstruction[2].u.operand;

    if (m_codeBlock->isConstantRegisterIndex(src)) {
        storePtr(ImmPtr(JSValue::encode(getConstantOperand(src))), Address(callFrameRegister, dst * sizeof(Register)));
        if (dst == m_lastResultBytecodeRegister)
            killLastResultRegister();
    } else if ((src == m_lastResultBytecodeRegister) || (dst == m_lastResultBytecodeRegister)) {
        // If either the src or dst is the cached register go though
        // get/put registers to make sure we track this correctly.
        emitGetVirtualRegister(src, regT0);
        emitPutVirtualRegister(dst);
    } else {
        // Perform the copy via regT1; do not disturb any mapping in regT0.
        loadPtr(Address(callFrameRegister, src * sizeof(Register)), regT1);
        storePtr(regT1, Address(callFrameRegister, dst * sizeof(Register)));
    }
}

void JIT::emit_op_end(Instruction* currentInstruction)
{
    if (m_codeBlock->needsFullScopeChain())
        JITStubCall(this, cti_op_end).call();
    ASSERT(returnValueRegister != callFrameRegister);
    emitGetVirtualRegister(currentInstruction[1].u.operand, returnValueRegister);
    restoreReturnAddressBeforeReturn(Address(callFrameRegister, RegisterFile::ReturnPC * static_cast<int>(sizeof(Register))));
    ret();
}

void JIT::emit_op_jmp(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[1].u.operand;
    addJump(jump(), target + 1);
    RECORD_JUMP_TARGET(target + 1);
}

void JIT::emit_op_loop(Instruction* currentInstruction)
{
    emitTimeoutCheck();

    unsigned target = currentInstruction[1].u.operand;
    addJump(jump(), target + 1);
}

void JIT::emit_op_loop_if_less(Instruction* currentInstruction)
{
    emitTimeoutCheck();

    unsigned op1 = currentInstruction[1].u.operand;
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;
    if (isOperandConstantImmediateInt(op2)) {
        emitGetVirtualRegister(op1, regT0);
        emitJumpSlowCaseIfNotImmediateInteger(regT0);
#if USE(JSVALUE64)
        int32_t op2imm = getConstantOperandImmediateInt(op2);
#else
        int32_t op2imm = static_cast<int32_t>(JSImmediate::rawValue(getConstantOperand(op2)));
#endif
        addJump(branch32(LessThan, regT0, Imm32(op2imm)), target + 3);
    } else if (isOperandConstantImmediateInt(op1)) {
        emitGetVirtualRegister(op2, regT0);
        emitJumpSlowCaseIfNotImmediateInteger(regT0);
#if USE(JSVALUE64)
        int32_t op1imm = getConstantOperandImmediateInt(op1);
#else
        int32_t op1imm = static_cast<int32_t>(JSImmediate::rawValue(getConstantOperand(op1)));
#endif
        addJump(branch32(GreaterThan, regT0, Imm32(op1imm)), target + 3);
    } else {
        emitGetVirtualRegisters(op1, regT0, op2, regT1);
        emitJumpSlowCaseIfNotImmediateInteger(regT0);
        emitJumpSlowCaseIfNotImmediateInteger(regT1);
        addJump(branch32(LessThan, regT0, regT1), target + 3);
    }
}

void JIT::emit_op_loop_if_lesseq(Instruction* currentInstruction)
{
    emitTimeoutCheck();

    unsigned op1 = currentInstruction[1].u.operand;
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;
    if (isOperandConstantImmediateInt(op2)) {
        emitGetVirtualRegister(op1, regT0);
        emitJumpSlowCaseIfNotImmediateInteger(regT0);
#if USE(JSVALUE64)
        int32_t op2imm = getConstantOperandImmediateInt(op2);
#else
        int32_t op2imm = static_cast<int32_t>(JSImmediate::rawValue(getConstantOperand(op2)));
#endif
        addJump(branch32(LessThanOrEqual, regT0, Imm32(op2imm)), target + 3);
    } else {
        emitGetVirtualRegisters(op1, regT0, op2, regT1);
        emitJumpSlowCaseIfNotImmediateInteger(regT0);
        emitJumpSlowCaseIfNotImmediateInteger(regT1);
        addJump(branch32(LessThanOrEqual, regT0, regT1), target + 3);
    }
}

void JIT::emit_op_new_object(Instruction* currentInstruction)
{
    JITStubCall(this, cti_op_new_object).call(currentInstruction[1].u.operand);
}

void JIT::emit_op_instanceof(Instruction* currentInstruction)
{
    // Load the operands (baseVal, proto, and value respectively) into registers.
    // We use regT0 for baseVal since we will be done with this first, and we can then use it for the result.
    emitGetVirtualRegister(currentInstruction[3].u.operand, regT0);
    emitGetVirtualRegister(currentInstruction[4].u.operand, regT1);
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT2);

    // Check that baseVal & proto are cells.
    emitJumpSlowCaseIfNotJSCell(regT0);
    emitJumpSlowCaseIfNotJSCell(regT1);

    // Check that baseVal is an object, that it 'ImplementsHasInstance' but that it does not 'OverridesHasInstance'.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT0);
    addSlowCase(branch32(NotEqual, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_type)), Imm32(ObjectType)));
    addSlowCase(branchTest32(Zero, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(ImplementsDefaultHasInstance)));

    // If value is not an Object, return false.
    Jump valueIsImmediate = emitJumpIfNotJSCell(regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSCell, m_structure)), regT0);
    Jump valueIsNotObject = branch32(NotEqual, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_type)), Imm32(ObjectType));

    // Check proto is object.
    loadPtr(Address(regT1, OBJECT_OFFSETOF(JSCell, m_structure)), regT0);
    addSlowCase(branch32(NotEqual, Address(regT0, OBJECT_OFFSETOF(Structure, m_typeInfo.m_type)), Imm32(ObjectType)));

    // Optimistically load the result true, and start looping.
    // Initially, regT1 still contains proto and regT2 still contains value.
    // As we loop regT2 will be updated with its prototype, recursively walking the prototype chain.
    move(ImmPtr(JSValue::encode(jsBoolean(true))), regT0);
    Label loop(this);

    // Load the prototype of the object in regT2.  If this is equal to regT1 - WIN!
    // Otherwise, check if we've hit null - if we have then drop out of the loop, if not go again.
    loadPtr(Address(regT2, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    loadPtr(Address(regT2, OBJECT_OFFSETOF(Structure, m_prototype)), regT2);
    Jump isInstance = branchPtr(Equal, regT2, regT1);
    branchPtr(NotEqual, regT2, ImmPtr(JSValue::encode(jsNull())), loop);

    // We get here either by dropping out of the loop, or if value was not an Object.  Result is false.
    valueIsImmediate.link(this);
    valueIsNotObject.link(this);
    move(ImmPtr(JSValue::encode(jsBoolean(false))), regT0);

    // isInstance jumps right down to here, to skip setting the result to false (it has already set true).
    isInstance.link(this);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_new_func(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_func);
    stubCall.addArgument(ImmPtr(m_codeBlock->function(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_call(Instruction* currentInstruction)
{
    compileOpCall(op_call, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_call_eval(Instruction* currentInstruction)
{
    compileOpCall(op_call_eval, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_load_varargs(Instruction* currentInstruction)
{
    int argCountDst = currentInstruction[1].u.operand;
    int argsOffset = currentInstruction[2].u.operand;

    JITStubCall stubCall(this, cti_op_load_varargs);
    stubCall.addArgument(Imm32(argsOffset));
    stubCall.call();
    // Stores a naked int32 in the register file.
    store32(returnValueRegister, Address(callFrameRegister, argCountDst * sizeof(Register)));
}

void JIT::emit_op_call_varargs(Instruction* currentInstruction)
{
    compileOpCallVarargs(currentInstruction);
}

void JIT::emit_op_construct(Instruction* currentInstruction)
{
    compileOpCall(op_construct, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_get_global_var(Instruction* currentInstruction)
{
    JSVariableObject* globalObject = static_cast<JSVariableObject*>(currentInstruction[2].u.jsCell);
    move(ImmPtr(globalObject), regT0);
    emitGetVariableObjectRegister(regT0, currentInstruction[3].u.operand, regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_put_global_var(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[3].u.operand, regT1);
    JSVariableObject* globalObject = static_cast<JSVariableObject*>(currentInstruction[1].u.jsCell);
    move(ImmPtr(globalObject), regT0);
    emitPutVariableObjectRegister(regT1, regT0, currentInstruction[2].u.operand);
}

void JIT::emit_op_get_scoped_var(Instruction* currentInstruction)
{
    int skip = currentInstruction[3].u.operand + m_codeBlock->needsFullScopeChain();

    emitGetFromCallFrameHeaderPtr(RegisterFile::ScopeChain, regT0);
    while (skip--)
        loadPtr(Address(regT0, OBJECT_OFFSETOF(ScopeChainNode, next)), regT0);

    loadPtr(Address(regT0, OBJECT_OFFSETOF(ScopeChainNode, object)), regT0);
    emitGetVariableObjectRegister(regT0, currentInstruction[2].u.operand, regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_put_scoped_var(Instruction* currentInstruction)
{
    int skip = currentInstruction[2].u.operand + m_codeBlock->needsFullScopeChain();

    emitGetFromCallFrameHeaderPtr(RegisterFile::ScopeChain, regT1);
    emitGetVirtualRegister(currentInstruction[3].u.operand, regT0);
    while (skip--)
        loadPtr(Address(regT1, OBJECT_OFFSETOF(ScopeChainNode, next)), regT1);

    loadPtr(Address(regT1, OBJECT_OFFSETOF(ScopeChainNode, object)), regT1);
    emitPutVariableObjectRegister(regT0, regT1, currentInstruction[1].u.operand);
}

void JIT::emit_op_tear_off_activation(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_tear_off_activation);
    stubCall.addArgument(currentInstruction[1].u.operand, regT2);
    stubCall.call();
}

void JIT::emit_op_tear_off_arguments(Instruction*)
{
    JITStubCall(this, cti_op_tear_off_arguments).call();
}

void JIT::emit_op_ret(Instruction* currentInstruction)
{
    // We could JIT generate the deref, only calling out to C when the refcount hits zero.
    if (m_codeBlock->needsFullScopeChain())
        JITStubCall(this, cti_op_ret_scopeChain).call();

    ASSERT(callFrameRegister != regT1);
    ASSERT(regT1 != returnValueRegister);
    ASSERT(returnValueRegister != callFrameRegister);

    // Return the result in %eax.
    emitGetVirtualRegister(currentInstruction[1].u.operand, returnValueRegister);

    // Grab the return address.
    emitGetFromCallFrameHeaderPtr(RegisterFile::ReturnPC, regT1);

    // Restore our caller's "r".
    emitGetFromCallFrameHeaderPtr(RegisterFile::CallerFrame, callFrameRegister);

    // Return.
    restoreReturnAddressBeforeReturn(regT1);
    ret();
}

void JIT::emit_op_new_array(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_array);
    stubCall.addArgument(Imm32(currentInstruction[2].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_resolve(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_construct_verify(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);

    emitJumpSlowCaseIfNotJSCell(regT0);
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    addSlowCase(branch32(NotEqual, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo) + OBJECT_OFFSETOF(TypeInfo, m_type)), Imm32(ObjectType)));

}

void JIT::emit_op_to_primitive(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src, regT0);
    
    Jump isImm = emitJumpIfNotJSCell(regT0);
    addSlowCase(branchPtr(NotEqual, Address(regT0), ImmPtr(m_globalData->jsStringVPtr)));
    isImm.link(this);

    if (dst != src)
        emitPutVirtualRegister(dst);

}

void JIT::emit_op_strcat(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_strcat);
    stubCall.addArgument(Imm32(currentInstruction[2].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_loop_if_true(Instruction* currentInstruction)
{
    emitTimeoutCheck();

    unsigned target = currentInstruction[2].u.operand;
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);

    Jump isZero = branchPtr(Equal, regT0, ImmPtr(JSValue::encode(jsNumber(m_globalData, 0))));
    addJump(emitJumpIfImmediateInteger(regT0), target + 2);

    addJump(branchPtr(Equal, regT0, ImmPtr(JSValue::encode(jsBoolean(true)))), target + 2);
    addSlowCase(branchPtr(NotEqual, regT0, ImmPtr(JSValue::encode(jsBoolean(false)))));

    isZero.link(this);
};
void JIT::emit_op_resolve_base(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve_base);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_resolve_skip(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve_skip);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand + m_codeBlock->needsFullScopeChain()));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_resolve_global(Instruction* currentInstruction)
{
    // Fast case
    void* globalObject = currentInstruction[2].u.jsCell;
    Identifier* ident = &m_codeBlock->identifier(currentInstruction[3].u.operand);
    
    unsigned currentIndex = m_globalResolveInfoIndex++;
    void* structureAddress = &(m_codeBlock->globalResolveInfo(currentIndex).structure);
    void* offsetAddr = &(m_codeBlock->globalResolveInfo(currentIndex).offset);

    // Check Structure of global object
    move(ImmPtr(globalObject), regT0);
    loadPtr(structureAddress, regT1);
    Jump noMatch = branchPtr(NotEqual, regT1, Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure))); // Structures don't match

    // Load cached property
    // Assume that the global object always uses external storage.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSGlobalObject, m_externalStorage)), regT0);
    load32(offsetAddr, regT1);
    loadPtr(BaseIndex(regT0, regT1, ScalePtr), regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
    Jump end = jump();

    // Slow case
    noMatch.link(this);
    JITStubCall stubCall(this, cti_op_resolve_global);
    stubCall.addArgument(ImmPtr(globalObject));
    stubCall.addArgument(ImmPtr(ident));
    stubCall.addArgument(Imm32(currentIndex));
    stubCall.call(currentInstruction[1].u.operand);
    end.link(this);
}

void JIT::emit_op_not(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT0);
    xorPtr(Imm32(static_cast<int32_t>(JSImmediate::FullTagTypeBool)), regT0);
    addSlowCase(branchTestPtr(NonZero, regT0, Imm32(static_cast<int32_t>(~JSImmediate::ExtendedPayloadBitBoolValue))));
    xorPtr(Imm32(static_cast<int32_t>(JSImmediate::FullTagTypeBool | JSImmediate::ExtendedPayloadBitBoolValue)), regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_jfalse(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[2].u.operand;
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);

    addJump(branchPtr(Equal, regT0, ImmPtr(JSValue::encode(jsNumber(m_globalData, 0)))), target + 2);
    Jump isNonZero = emitJumpIfImmediateInteger(regT0);

    addJump(branchPtr(Equal, regT0, ImmPtr(JSValue::encode(jsBoolean(false)))), target + 2);
    addSlowCase(branchPtr(NotEqual, regT0, ImmPtr(JSValue::encode(jsBoolean(true)))));

    isNonZero.link(this);
    RECORD_JUMP_TARGET(target + 2);
};
void JIT::emit_op_jeq_null(Instruction* currentInstruction)
{
    unsigned src = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    // First, handle JSCell cases - check MasqueradesAsUndefined bit on the structure.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    addJump(branchTest32(NonZero, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined)), target + 2);
    Jump wasNotImmediate = jump();

    // Now handle the immediate cases - undefined & null
    isImmediate.link(this);
    andPtr(Imm32(~JSImmediate::ExtendedTagBitUndefined), regT0);
    addJump(branchPtr(Equal, regT0, ImmPtr(JSValue::encode(jsNull()))), target + 2);            

    wasNotImmediate.link(this);
    RECORD_JUMP_TARGET(target + 2);
};
void JIT::emit_op_jneq_null(Instruction* currentInstruction)
{
    unsigned src = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    // First, handle JSCell cases - check MasqueradesAsUndefined bit on the structure.
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    addJump(branchTest32(Zero, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined)), target + 2);
    Jump wasNotImmediate = jump();

    // Now handle the immediate cases - undefined & null
    isImmediate.link(this);
    andPtr(Imm32(~JSImmediate::ExtendedTagBitUndefined), regT0);
    addJump(branchPtr(NotEqual, regT0, ImmPtr(JSValue::encode(jsNull()))), target + 2);            

    wasNotImmediate.link(this);
    RECORD_JUMP_TARGET(target + 2);
}

void JIT::emit_op_jneq_ptr(Instruction* currentInstruction)
{
    unsigned src = currentInstruction[1].u.operand;
    JSCell* ptr = currentInstruction[2].u.jsCell;
    unsigned target = currentInstruction[3].u.operand;
    
    emitGetVirtualRegister(src, regT0);
    addJump(branchPtr(NotEqual, regT0, ImmPtr(JSValue::encode(JSValue(ptr)))), target + 3);            

    RECORD_JUMP_TARGET(target + 3);
}

void JIT::emit_op_jsr(Instruction* currentInstruction)
{
    int retAddrDst = currentInstruction[1].u.operand;
    int target = currentInstruction[2].u.operand;
    DataLabelPtr storeLocation = storePtrWithPatch(ImmPtr(0), Address(callFrameRegister, sizeof(Register) * retAddrDst));
    addJump(jump(), target + 2);
    m_jsrSites.append(JSRInfo(storeLocation, label()));
    killLastResultRegister();
    RECORD_JUMP_TARGET(target + 2);
}

void JIT::emit_op_sret(Instruction* currentInstruction)
{
    jump(Address(callFrameRegister, sizeof(Register) * currentInstruction[1].u.operand));
    killLastResultRegister();
}

void JIT::emit_op_eq(Instruction* currentInstruction)
{
    emitGetVirtualRegisters(currentInstruction[2].u.operand, regT0, currentInstruction[3].u.operand, regT1);
    emitJumpSlowCaseIfNotImmediateIntegers(regT0, regT1, regT2);
    set32(Equal, regT1, regT0, regT0);
    emitTagAsBoolImmediate(regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_bitnot(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT0);
    emitJumpSlowCaseIfNotImmediateInteger(regT0);
#if USE(JSVALUE64)
    not32(regT0);
    emitFastArithIntToImmNoCheck(regT0, regT0);
#else
    xorPtr(Imm32(~JSImmediate::TagTypeNumber), regT0);
#endif
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_resolve_with_base(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_resolve_with_base);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[3].u.operand)));
    stubCall.addArgument(Imm32(currentInstruction[1].u.operand));
    stubCall.call(currentInstruction[2].u.operand);
}

void JIT::emit_op_new_func_exp(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_func_exp);
    stubCall.addArgument(ImmPtr(m_codeBlock->functionExpression(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_jtrue(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[2].u.operand;
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);

    Jump isZero = branchPtr(Equal, regT0, ImmPtr(JSValue::encode(jsNumber(m_globalData, 0))));
    addJump(emitJumpIfImmediateInteger(regT0), target + 2);

    addJump(branchPtr(Equal, regT0, ImmPtr(JSValue::encode(jsBoolean(true)))), target + 2);
    addSlowCase(branchPtr(NotEqual, regT0, ImmPtr(JSValue::encode(jsBoolean(false)))));

    isZero.link(this);
    RECORD_JUMP_TARGET(target + 2);
}

void JIT::emit_op_neq(Instruction* currentInstruction)
{
    emitGetVirtualRegisters(currentInstruction[2].u.operand, regT0, currentInstruction[3].u.operand, regT1);
    emitJumpSlowCaseIfNotImmediateIntegers(regT0, regT1, regT2);
    set32(NotEqual, regT1, regT0, regT0);
    emitTagAsBoolImmediate(regT0);

    emitPutVirtualRegister(currentInstruction[1].u.operand);

}

void JIT::emit_op_bitxor(Instruction* currentInstruction)
{
    emitGetVirtualRegisters(currentInstruction[2].u.operand, regT0, currentInstruction[3].u.operand, regT1);
    emitJumpSlowCaseIfNotImmediateIntegers(regT0, regT1, regT2);
    xorPtr(regT1, regT0);
    emitFastArithReTagImmediate(regT0, regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_new_regexp(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_regexp);
    stubCall.addArgument(ImmPtr(m_codeBlock->regexp(currentInstruction[2].u.operand)));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_bitor(Instruction* currentInstruction)
{
    emitGetVirtualRegisters(currentInstruction[2].u.operand, regT0, currentInstruction[3].u.operand, regT1);
    emitJumpSlowCaseIfNotImmediateIntegers(regT0, regT1, regT2);
    orPtr(regT1, regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_throw(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_throw);
    stubCall.addArgument(currentInstruction[1].u.operand, regT2);
    stubCall.call();
    ASSERT(regT0 == returnValueRegister);
#ifndef NDEBUG
    // cti_op_throw always changes it's return address,
    // this point in the code should never be reached.
    breakpoint();
#endif
}

void JIT::emit_op_next_pname(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_next_pname);
    stubCall.addArgument(currentInstruction[2].u.operand, regT2);
    stubCall.call();
    Jump endOfIter = branchTestPtr(Zero, regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
    addJump(jump(), currentInstruction[3].u.operand + 3);
    endOfIter.link(this);
}

void JIT::emit_op_push_scope(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_push_scope);
    stubCall.addArgument(currentInstruction[1].u.operand, regT2);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_pop_scope(Instruction*)
{
    JITStubCall(this, cti_op_pop_scope).call();
}

void JIT::compileOpStrictEq(Instruction* currentInstruction, CompileOpStrictEqType type)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;
    unsigned src2 = currentInstruction[3].u.operand;

    emitGetVirtualRegisters(src1, regT0, src2, regT1);

    // Jump to a slow case if either operand is a number, or if both are JSCell*s.
    move(regT0, regT2);
    orPtr(regT1, regT2);
    addSlowCase(emitJumpIfJSCell(regT2));
    addSlowCase(emitJumpIfImmediateNumber(regT2));

    if (type == OpStrictEq)
        set32(Equal, regT1, regT0, regT0);
    else
        set32(NotEqual, regT1, regT0, regT0);
    emitTagAsBoolImmediate(regT0);

    emitPutVirtualRegister(dst);
}

void JIT::emit_op_stricteq(Instruction* currentInstruction)
{
    compileOpStrictEq(currentInstruction, OpStrictEq);
}

void JIT::emit_op_nstricteq(Instruction* currentInstruction)
{
    compileOpStrictEq(currentInstruction, OpNStrictEq);
}

void JIT::emit_op_to_jsnumber(Instruction* currentInstruction)
{
    int srcVReg = currentInstruction[2].u.operand;
    emitGetVirtualRegister(srcVReg, regT0);
    
    Jump wasImmediate = emitJumpIfImmediateInteger(regT0);

    emitJumpSlowCaseIfNotJSCell(regT0, srcVReg);
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    addSlowCase(branch32(NotEqual, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_type)), Imm32(NumberType)));
    
    wasImmediate.link(this);

    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_push_new_scope(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_push_new_scope);
    stubCall.addArgument(ImmPtr(&m_codeBlock->identifier(currentInstruction[2].u.operand)));
    stubCall.addArgument(currentInstruction[3].u.operand, regT2);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_catch(Instruction* currentInstruction)
{
    killLastResultRegister(); // FIXME: Implicitly treat op_catch as a labeled statement, and remove this line of code.
    peek(callFrameRegister, OBJECT_OFFSETOF(struct JITStackFrame, callFrame) / sizeof (void*));
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_jmp_scopes(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_jmp_scopes);
    stubCall.addArgument(Imm32(currentInstruction[1].u.operand));
    stubCall.call();
    addJump(jump(), currentInstruction[2].u.operand + 2);
    RECORD_JUMP_TARGET(currentInstruction[2].u.operand + 2);
}

void JIT::emit_op_switch_imm(Instruction* currentInstruction)
{
    unsigned tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    SimpleJumpTable* jumpTable = &m_codeBlock->immediateSwitchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeIndex, defaultOffset, SwitchRecord::Immediate));
    jumpTable->ctiOffsets.grow(jumpTable->branchOffsets.size());

    JITStubCall stubCall(this, cti_op_switch_imm);
    stubCall.addArgument(scrutinee, regT2);
    stubCall.addArgument(Imm32(tableIndex));
    stubCall.call();
    jump(regT0);
}

void JIT::emit_op_switch_char(Instruction* currentInstruction)
{
    unsigned tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    SimpleJumpTable* jumpTable = &m_codeBlock->characterSwitchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeIndex, defaultOffset, SwitchRecord::Character));
    jumpTable->ctiOffsets.grow(jumpTable->branchOffsets.size());

    JITStubCall stubCall(this, cti_op_switch_char);
    stubCall.addArgument(scrutinee, regT2);
    stubCall.addArgument(Imm32(tableIndex));
    stubCall.call();
    jump(regT0);
}

void JIT::emit_op_switch_string(Instruction* currentInstruction)
{
    unsigned tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    StringJumpTable* jumpTable = &m_codeBlock->stringSwitchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeIndex, defaultOffset));

    JITStubCall stubCall(this, cti_op_switch_string);
    stubCall.addArgument(scrutinee, regT2);
    stubCall.addArgument(Imm32(tableIndex));
    stubCall.call();
    jump(regT0);
}

void JIT::emit_op_new_error(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_new_error);
    stubCall.addArgument(Imm32(currentInstruction[2].u.operand));
    stubCall.addArgument(ImmPtr(JSValue::encode(m_codeBlock->getConstant(currentInstruction[3].u.operand))));
    stubCall.addArgument(Imm32(m_bytecodeIndex));
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emit_op_debug(Instruction* currentInstruction)
{
    JITStubCall stubCall(this, cti_op_debug);
    stubCall.addArgument(Imm32(currentInstruction[1].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[2].u.operand));
    stubCall.addArgument(Imm32(currentInstruction[3].u.operand));
    stubCall.call();
}

void JIT::emit_op_eq_null(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src1, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    setTest32(NonZero, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined), regT0);

    Jump wasNotImmediate = jump();

    isImmediate.link(this);

    andPtr(Imm32(~JSImmediate::ExtendedTagBitUndefined), regT0);
    setPtr(Equal, regT0, Imm32(JSImmediate::FullTagTypeNull), regT0);

    wasNotImmediate.link(this);

    emitTagAsBoolImmediate(regT0);
    emitPutVirtualRegister(dst);

}

void JIT::emit_op_neq_null(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;
    unsigned src1 = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src1, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT2);
    setTest32(Zero, Address(regT2, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(MasqueradesAsUndefined), regT0);

    Jump wasNotImmediate = jump();

    isImmediate.link(this);

    andPtr(Imm32(~JSImmediate::ExtendedTagBitUndefined), regT0);
    setPtr(NotEqual, regT0, Imm32(JSImmediate::FullTagTypeNull), regT0);

    wasNotImmediate.link(this);

    emitTagAsBoolImmediate(regT0);
    emitPutVirtualRegister(dst);

}

void JIT::emit_op_enter(Instruction*)
{
    // Even though CTI doesn't use them, we initialize our constant
    // registers to zap stale pointers, to avoid unnecessarily prolonging
    // object lifetime and increasing GC pressure.
    size_t count = m_codeBlock->m_numVars;
    for (size_t j = 0; j < count; ++j)
        emitInitRegister(j);

}

void JIT::emit_op_enter_with_activation(Instruction* currentInstruction)
{
    // Even though CTI doesn't use them, we initialize our constant
    // registers to zap stale pointers, to avoid unnecessarily prolonging
    // object lifetime and increasing GC pressure.
    size_t count = m_codeBlock->m_numVars;
    for (size_t j = 0; j < count; ++j)
        emitInitRegister(j);

    JITStubCall(this, cti_op_push_activation).call(currentInstruction[1].u.operand);
}

void JIT::emit_op_create_arguments(Instruction*)
{
    Jump argsCreated = branchTestPtr(NonZero, Address(callFrameRegister, sizeof(Register) * RegisterFile::ArgumentsRegister));
    if (m_codeBlock->m_numParameters == 1)
        JITStubCall(this, cti_op_create_arguments_no_params).call();
    else
        JITStubCall(this, cti_op_create_arguments).call();
    argsCreated.link(this);
}
    
void JIT::emit_op_init_arguments(Instruction*)
{
    storePtr(ImmPtr(0), Address(callFrameRegister, sizeof(Register) * RegisterFile::ArgumentsRegister));
}

void JIT::emit_op_convert_this(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);

    emitJumpSlowCaseIfNotJSCell(regT0);
    loadPtr(Address(regT0, OBJECT_OFFSETOF(JSCell, m_structure)), regT1);
    addSlowCase(branchTest32(NonZero, Address(regT1, OBJECT_OFFSETOF(Structure, m_typeInfo.m_flags)), Imm32(NeedsThisConversion)));

}

void JIT::emit_op_profile_will_call(Instruction* currentInstruction)
{
    peek(regT1, OBJECT_OFFSETOF(JITStackFrame, enabledProfilerReference) / sizeof (void*));
    Jump noProfiler = branchTestPtr(Zero, Address(regT1));

    JITStubCall stubCall(this, cti_op_profile_will_call);
    stubCall.addArgument(currentInstruction[1].u.operand, regT1);
    stubCall.call();
    noProfiler.link(this);

}

void JIT::emit_op_profile_did_call(Instruction* currentInstruction)
{
    peek(regT1, OBJECT_OFFSETOF(JITStackFrame, enabledProfilerReference) / sizeof (void*));
    Jump noProfiler = branchTestPtr(Zero, Address(regT1));

    JITStubCall stubCall(this, cti_op_profile_did_call);
    stubCall.addArgument(currentInstruction[1].u.operand, regT1);
    stubCall.call();
    noProfiler.link(this);
}


// Slow cases

void JIT::emitSlow_op_convert_this(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_convert_this);
    stubCall.addArgument(regT0);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_construct_verify(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_to_primitive(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_to_primitive);
    stubCall.addArgument(regT0);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_get_by_val(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    // The slow void JIT::emitSlow_that handles accesses to arrays (below) may jump back up to here. 
    Label beginGetByValSlow(this);

    Jump notImm = getSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    emitFastArithIntToImmNoCheck(regT1, regT1);

    notImm.link(this);
    JITStubCall stubCall(this, cti_op_get_by_val);
    stubCall.addArgument(regT0);
    stubCall.addArgument(regT1);
    stubCall.call(currentInstruction[1].u.operand);
    emitJumpSlowToHot(jump(), OPCODE_LENGTH(op_get_by_val));

    // This is slow void JIT::emitSlow_that handles accesses to arrays above the fast cut-off.
    // First, check if this is an access to the vector
    linkSlowCase(iter);
    branch32(AboveOrEqual, regT1, Address(regT2, OBJECT_OFFSETOF(ArrayStorage, m_vectorLength)), beginGetByValSlow);

    // okay, missed the fast region, but it is still in the vector.  Get the value.
    loadPtr(BaseIndex(regT2, regT1, ScalePtr, OBJECT_OFFSETOF(ArrayStorage, m_vector[0])), regT2);
    // Check whether the value loaded is zero; if so we need to return undefined.
    branchTestPtr(Zero, regT2, beginGetByValSlow);
    move(regT2, regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand, regT0);
}

void JIT::emitSlow_op_loop_if_less(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned op1 = currentInstruction[1].u.operand;
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;
    if (isOperandConstantImmediateInt(op2)) {
        linkSlowCase(iter);
        JITStubCall stubCall(this, cti_op_loop_if_less);
        stubCall.addArgument(regT0);
        stubCall.addArgument(op2, regT2);
        stubCall.call();
        emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 3);
    } else if (isOperandConstantImmediateInt(op1)) {
        linkSlowCase(iter);
        JITStubCall stubCall(this, cti_op_loop_if_less);
        stubCall.addArgument(op1, regT2);
        stubCall.addArgument(regT0);
        stubCall.call();
        emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 3);
    } else {
        linkSlowCase(iter);
        linkSlowCase(iter);
        JITStubCall stubCall(this, cti_op_loop_if_less);
        stubCall.addArgument(regT0);
        stubCall.addArgument(regT1);
        stubCall.call();
        emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 3);
    }
}

void JIT::emitSlow_op_loop_if_lesseq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    unsigned op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;
    if (isOperandConstantImmediateInt(op2)) {
        linkSlowCase(iter);
        JITStubCall stubCall(this, cti_op_loop_if_lesseq);
        stubCall.addArgument(regT0);
        stubCall.addArgument(currentInstruction[2].u.operand, regT2);
        stubCall.call();
        emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 3);
    } else {
        linkSlowCase(iter);
        linkSlowCase(iter);
        JITStubCall stubCall(this, cti_op_loop_if_lesseq);
        stubCall.addArgument(regT0);
        stubCall.addArgument(regT1);
        stubCall.call();
        emitJumpSlowToHot(branchTest32(NonZero, regT0), target + 3);
    }
}

void JIT::emitSlow_op_put_by_val(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    // Normal slow cases - either is not an immediate imm, or is an array.
    Jump notImm = getSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    emitFastArithIntToImmNoCheck(regT1, regT1);

    notImm.link(this); {
        JITStubCall stubCall(this, cti_op_put_by_val);
        stubCall.addArgument(regT0);
        stubCall.addArgument(regT1);
        stubCall.addArgument(currentInstruction[3].u.operand, regT2);
        stubCall.call();
        emitJumpSlowToHot(jump(), OPCODE_LENGTH(op_put_by_val));
    }

    // slow cases for immediate int accesses to arrays
    linkSlowCase(iter);
    linkSlowCase(iter); {
        JITStubCall stubCall(this, cti_op_put_by_val_array);
        stubCall.addArgument(regT0);
        stubCall.addArgument(regT1);
        stubCall.addArgument(currentInstruction[3].u.operand, regT2);
        stubCall.call();
    }
}

void JIT::emitSlow_op_loop_if_true(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_jtrue);
    stubCall.addArgument(regT0);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(NonZero, regT0), currentInstruction[2].u.operand + 2);
}

void JIT::emitSlow_op_not(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    xorPtr(Imm32(static_cast<int32_t>(JSImmediate::FullTagTypeBool)), regT0);
    JITStubCall stubCall(this, cti_op_not);
    stubCall.addArgument(regT0);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_jfalse(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_jtrue);
    stubCall.addArgument(regT0);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(Zero, regT0), currentInstruction[2].u.operand + 2); // inverted!
}

void JIT::emitSlow_op_bitnot(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_bitnot);
    stubCall.addArgument(regT0);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_jtrue(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_jtrue);
    stubCall.addArgument(regT0);
    stubCall.call();
    emitJumpSlowToHot(branchTest32(NonZero, regT0), currentInstruction[2].u.operand + 2);
}

void JIT::emitSlow_op_bitxor(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_bitxor);
    stubCall.addArgument(regT0);
    stubCall.addArgument(regT1);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_bitor(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_bitor);
    stubCall.addArgument(regT0);
    stubCall.addArgument(regT1);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_eq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_eq);
    stubCall.addArgument(regT0);
    stubCall.addArgument(regT1);
    stubCall.call();
    emitTagAsBoolImmediate(regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_neq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_eq);
    stubCall.addArgument(regT0);
    stubCall.addArgument(regT1);
    stubCall.call();
    xor32(Imm32(0x1), regT0);
    emitTagAsBoolImmediate(regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_stricteq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_stricteq);
    stubCall.addArgument(regT0);
    stubCall.addArgument(regT1);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_nstricteq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_nstricteq);
    stubCall.addArgument(regT0);
    stubCall.addArgument(regT1);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_instanceof(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITStubCall stubCall(this, cti_op_instanceof);
    stubCall.addArgument(currentInstruction[2].u.operand, regT2);
    stubCall.addArgument(currentInstruction[3].u.operand, regT2);
    stubCall.addArgument(currentInstruction[4].u.operand, regT2);
    stubCall.call(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_call(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(currentInstruction, iter, m_callLinkInfoIndex++, op_call);
}

void JIT::emitSlow_op_call_eval(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(currentInstruction, iter, m_callLinkInfoIndex++, op_call_eval);
}

void JIT::emitSlow_op_call_varargs(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallVarargsSlowCase(currentInstruction, iter);
}

void JIT::emitSlow_op_construct(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(currentInstruction, iter, m_callLinkInfoIndex++, op_construct);
}

void JIT::emitSlow_op_to_jsnumber(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCaseIfNotJSCell(iter, currentInstruction[2].u.operand);
    linkSlowCase(iter);

    JITStubCall stubCall(this, cti_op_to_jsnumber);
    stubCall.addArgument(regT0);
    stubCall.call(currentInstruction[1].u.operand);
}

#endif // USE(JSVALUE32_64)

} // namespace JSC

#endif // ENABLE(JIT)
