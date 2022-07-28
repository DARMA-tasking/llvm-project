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

  // This fixes an error:
  //
  // const clang::ASTRecordLayout& clang::ASTContext::getASTRecordLayout(const
  // clang::RecordDecl*) const: Assertion `D && "Cannot get layout of forward
  // declarations!"' failed.
  //
  // For example, it comes from declaration
  // `std::string prettyPrintStack(DumpStackType const& stack);`
  // from src/vt/configs/error/stack_out.h
  const auto *FuncDecl =
      dyn_cast_or_null<FunctionDecl>(ConstRefParm->getParentFunctionOrMethod());
  if (!FuncDecl || !FuncDecl->isThisDeclarationADefinition()) {
    return;
  }
        ConstRefParmType.getNonReferenceType().getUnqualifiedType();
    const auto ParmWidth = Result.Context->getTypeInfo(ParmType).Width;
    const auto Hint =
        FixItHint::CreateReplacement(ConstRefParm->getSourceRange(),
                                     "const " + ParmType.getAsString() + " " +
                                         ConstRefParm->getNameAsString());
    diag(ConstRefParm->getBeginLoc(), "passing small (%0 bits) variable by "
                                      "const ref, consider passing by value")
        << ParmWidth << ConstRefParm << Hint;
  }
}

} // namespace misc
} // namespace tidy
} // namespace clang
