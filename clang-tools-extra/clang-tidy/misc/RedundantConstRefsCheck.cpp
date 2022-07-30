//===--- RedundantConstRefsCheck.cpp - clang-tidy -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RedundantConstRefsCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace misc {

void RedundantConstRefsCheck::registerMatchers(MatchFinder *Finder) {
  auto ConstRefParm =
      parmVarDecl(hasType(qualType(lValueReferenceType(),
                                   references(isConstQualified()))),
                  unless(isExpansionInSystemHeader()))
          .bind("const-ref-parm");

  Finder->addMatcher(ConstRefParm, this);
}

void RedundantConstRefsCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *ConstRefParm =
      Result.Nodes.getNodeAs<ParmVarDecl>("const-ref-parm");

  const auto *FuncDecl =
      dyn_cast_or_null<FunctionDecl>(ConstRefParm->getParentFunctionOrMethod());
  if (!FuncDecl) {
    return;
  }

  // `!FuncDecl->isThisDeclarationADefinition()` fixes an error:
  //
  // const clang::ASTRecordLayout& clang::ASTContext::getASTRecordLayout(const
  // clang::RecordDecl*) const: Assertion `D && "Cannot get layout of forward
  // declarations!"' failed.
  //
  // For example, it comes from declaration
  // `std::string prettyPrintStack(DumpStackType const& stack);`
  // from src/vt/configs/error/stack_out.h
  if (!FuncDecl->isThisDeclarationADefinition()) {
    return;
  }

  // Ignore template functions specializations
  if (FuncDecl->isFunctionTemplateSpecialization()) {
    return;
  }

  const auto ParmType =
      ConstRefParm->getType().getNonReferenceType().getUnqualifiedType();

  // This fixes an error:
  //
  // clang::TypeInfo clang::ASTContext::getTypeInfoImpl(const clang::Type*)
  // const: Assertion `!T->isDependentType() && "should not see dependent types
  // here"' failed.
  //
  // For example, it comes from declaration
  // std::string prettyPrintStack(DumpStackType const& stack);
  // from src/vt/configs/error/stack_out.h
  if (ParmType->isDependentType()) {
    return;
  }

  // This fixes an error:
  //
  // const clang::ASTRecordLayout& clang::ASTContext::getASTRecordLayout(const
  // clang::RecordDecl*) const: Assertion `D && "Cannot get layout of forward
  // declarations!"' failed.
  //
  // For example, it comes from declaration
  // template <typename SeqTag, template <typename> class SeqTrigger>
  // void TaggedSequencer<SeqTag, SeqTrigger>::sequenced(
  //   SeqType const& seq_id, UserSeqFunWithIDType const& fn
  // ) {
  // from src/vt/sequence/sequencer.impl.h
  if (ParmType->isIncompleteType()) {
    return;
  }

  const auto ParmWidth = Result.Context->getTypeInfo(ParmType).Width;
  if (ParmWidth <= 64) {
    const auto Hint =
        FixItHint::CreateReplacement(ConstRefParm->getSourceRange(),
                                     "const " + ParmType.getAsString() + " " +
                                         ConstRefParm->getNameAsString());
    diag(ConstRefParm->getBeginLoc(),
         "passing %0 (%1 bits) by const ref, consider passing by value")
        << ParmType << ParmWidth << ConstRefParm << Hint;
  }
}

} // namespace misc
} // namespace tidy
} // namespace clang
