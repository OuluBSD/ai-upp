#ifndef _Vfs_Builtin_h_
#define _Vfs_Builtin_h_



#define CURSOR_LIST \
	CURSOR(Null					, =, 0) \
	CURSOR(Namespace			, =,CXCursor_Namespace) \
	CURSOR(TypedefDecl			, =,CXCursor_TypedefDecl) \
	CURSOR(ClassDecl			, =,CXCursor_ClassDecl) \
	CURSOR(ClassTemplate		, =,CXCursor_ClassTemplate) \
	CURSOR(CXXMethod			, =,CXCursor_CXXMethod) \
	CURSOR(VarDecl				, =,CXCursor_VarDecl) \
	CURSOR(ParmDecl				, =,CXCursor_ParmDecl) \
	CURSOR(CompoundStmt			, =,CXCursor_CompoundStmt) \
	CURSOR(TranslationUnit		, =,CXCursor_TranslationUnit) \
	\
	CURSOR(IfStmt				, =,CXCursor_IfStmt) \
	CURSOR(DoStmt				, =,CXCursor_DoStmt) \
	CURSOR(WhileStmt			, =,CXCursor_WhileStmt) \
	CURSOR(ForStmt				, =,CXCursor_ForStmt) \
	CURSOR(BreakStmt			, =,CXCursor_BreakStmt) \
	CURSOR(ContinueStmt			, =,CXCursor_ContinueStmt) \
	CURSOR(CaseStmt				, =,CXCursor_CaseStmt) \
	CURSOR(DefaultStmt			, =,CXCursor_DefaultStmt) \
	CURSOR(ReturnStmt			, =,CXCursor_ReturnStmt) \
	CURSOR(SwitchStmt			, =,CXCursor_SwitchStmt) \
	CURSOR(BlockExpr			, =,CXCursor_BlockExpr) \
	\
	CURSOR(Vfs					, =,2000) \
	\
	/* todo Following names could be improved and converted to CXCursor values (easy or hard way) */ \
	CURSOR(Builtin,,) \
	CURSOR(MetaStmt,,) \
	CURSOR(StaticFunction,,) \
	CURSOR(Stmt,,) \
	/*CURSOR(Expr,,)*/ \
	CURSOR(Literal,,) \
	CURSOR(Literal_BOOL,,) \
	CURSOR(Literal_INT32,,) \
	CURSOR(Literal_INT64,,) \
	CURSOR(Literal_DOUBLE,,) \
	CURSOR(Literal_STRING,,) \
	CURSOR(NamePart,,) \
	CURSOR(Resolve,,) \
	CURSOR(Argument,,) \
	CURSOR(ArgumentList,,) \
	CURSOR(FunctionBuiltin,,) \
	CURSOR(MachineDecl,,) \
	CURSOR(MachineStmt,,) \
	CURSOR(ChainDecl,,) \
	CURSOR(ChainStmt,,) \
	CURSOR(LoopDecl,,) \
	CURSOR(LoopStmt,,) \
	CURSOR(MetaVariable,,) \
	CURSOR(MetaParameter,,) \
	CURSOR(MetaBuiltin,,) \
	CURSOR(MetaStaticFunction,,) \
	CURSOR(WorldStmt,,) \
	CURSOR(EntityStmt,,) \
	CURSOR(ComponentStmt,,) \
	CURSOR(SystemStmt,,) \
	CURSOR(PoolStmt,,) \
	CURSOR(AtomStmt,,) \
	CURSOR(CallArg,,) \
	CURSOR(Unresolved,,) \
	CURSOR(MetaClass,,) \
	CURSOR(Rval,,) \
	CURSOR(Ctor,,) \
	CURSOR(ArraySize,,) \
	CURSOR(TypePointer,,) \
	CURSOR(TypeLref,,) \
	CURSOR(MetaRval,,) \
	CURSOR(MetaCtor,,) \
	CURSOR(Object,,) \
	CURSOR(MetaResolve,,) \
	CURSOR(StateStmt,,) \
	CURSOR(DriverStmt,,) \
	CURSOR(EngineStmt,,) \
	CURSOR(SymlinkStmt,,) \
	\
	CURSOR(ValueDecl,,)				/* VarDecl | ParmDecl | Literal,,)  */ \
	CURSOR(TypeDecl,,)				/* Builtin | TypedefDecl | ClassDecl | SEMT_CLASS |  */ \
									/* ClassTemplate | TypePointer | TypeLref,,)  */ \
	CURSOR(Function,,)				/* StaticFunction | CXXMethod | FunctionBuiltin,,)  */ \
	CURSOR(Undefined,,)				/* Null | NamePart,,)  */ \
	CURSOR(ClassPath_ParmDecl,,)	/* ParmDecl | NamePart,,)  */ \
	CURSOR(ClassPath_VarDecl,,)		/* VarDecl | NamePart,,)  */ \
	CURSOR(ClassPath,,)				/* ClassPath_ParmDecl | ClassPath_VarDecl | Namespace | Function | SEMT_CLASS,,) */ \
	CURSOR(Compounding,,)			/* TranslationUnit | Namespace | CompoundStmt,,)  */ \
	CURSOR(WithRvalReturn,,)		/* Rval | Expr | Literal | Resolve | ArgumentList | Ctor | Object,,) */ \
	\
	CURSOR(EcsStmt,,)				/* EngineStmt | WorldStmt | EntityStmt | ComponentStmt | SystemStmt | PoolStmt,,) */ \
	\
	/* todo merge CURSOR(OldEcsStmt to CURSOR(EcsStmt */ \
	CURSOR(OldEcsStmt,,)			/* MachineDecl | MachineStmt | ChainDecl | ChainStmt | LoopDecl | DriverStmt | LoopStmt | StateStmt | AtomStmt,,) */ \
	\
	CURSOR(MetaValueDecl,,)			/* MetaVariable | MetaParameter,,) */ \
	CURSOR(MetaTypeDecl,,)			/* MetaBuiltin,,) */ \
	CURSOR(MetaFunction,,)			/* MetaStaticFunction | MetaFunction_METHOD | MetaFunction_BUILTIN,,) */ \
	CURSOR(ClassPath_MetaParam,,)	/* MetaParameter | NamePart,,) */ \
	CURSOR(ClassPath_MetaVar,,)		/* MetaVariable | NamePart,,) */ \
	\
	CURSOR(MetaDecl,,)				/* NamePart | MetaValueDecl | MetaTypeDecl | MetaFunction | MetaRval | MetaCtor | MetaResolve,,) */ \
	CURSOR(ClassPath_MetaDecl,,)	/* ClassPath_MetaParam | ClassPath_MetaVar | MetaFunction | MetaClass,,) */ \
	CURSOR(ExprCastable,,)			/* Cursor_VarDecl | Cursor_ParmDecl | Cursor_Literal | Cursor_Object | */ \
	CURSOR(ExprOp,,)				/* Cursor_ExprCastable | Cursor_Op */ \
	\
	CURSOR(Vfs_OpBegin			, =,3000) \
	\
	CURSOR(ElseStmt,,) \
	CURSOR(ForStmt_Conditional,,) \
	CURSOR(ForStmt_PostOp,,) \
	CURSOR(ForStmt_Range,,) \
	\
	CURSOR(MetaBlockExpr,,) \
	CURSOR(ExprStmt,,) \
	CURSOR(AtomConnectorStmt,,) \
	CURSOR(CtorStmt,,) \
	\
	CURSOR(MetaIfStmt,,) \
	CURSOR(MetaElseStmt,,) \
	CURSOR(MetaDoStmt,,) \
	CURSOR(MetaWhileStmt,,) \
	CURSOR(MetaForStmt,,) \
	CURSOR(MetaForStmt_Conditional,,) \
	CURSOR(MetaForStmt_Post,,) \
	CURSOR(MetaForStmt_Range,,) \
	CURSOR(MetaBreakStmt,,) \
	CURSOR(MetaContinueStmt,,) \
	CURSOR(MetaCaseStmt,,) \
	CURSOR(MetaDefaultStmt,,) \
	CURSOR(MetaReturnStmt,,) \
	CURSOR(MetaSwitchStmt,,) \
	CURSOR(MetaBlockStmt,,) \
	CURSOR(MetaExprStmt,,) \
	\
	CURSOR(Vfs_ExprBegin		, =,4000) \
	\
	CURSOR(Op,,) \
	CURSOR(Op_NULL,,) \
	CURSOR(Op_INC,,) \
	CURSOR(Op_DEC,,) \
	CURSOR(Op_POSTINC,,) \
	CURSOR(Op_POSTDEC,,) \
	CURSOR(Op_NEGATIVE,,) \
	CURSOR(Op_POSITIVE,,) \
	CURSOR(Op_NOT,,) \
	CURSOR(Op_NEGATE,,) \
	CURSOR(Op_ADD,,) \
	CURSOR(Op_SUB,,) \
	CURSOR(Op_MUL,,) \
	CURSOR(Op_DIV,,) \
	CURSOR(Op_MOD,,) \
	CURSOR(Op_LSH,,) \
	CURSOR(Op_RSH,,) \
	CURSOR(Op_GREQ,,) \
	CURSOR(Op_LSEQ,,) \
	CURSOR(Op_GREATER,,) \
	CURSOR(Op_LESS,,) \
	CURSOR(Op_EQ,,) \
	CURSOR(Op_INEQ,,) \
	CURSOR(Op_BWAND,,) \
	CURSOR(Op_BWXOR,,) \
	CURSOR(Op_BWOR,,) \
	CURSOR(Op_AND,,) \
	CURSOR(Op_OR,,) \
	CURSOR(Op_COND,,) \
	CURSOR(Op_ASSIGN,,) \
	CURSOR(Op_ADDASS,,) \
	CURSOR(Op_SUBASS,,) \
	CURSOR(Op_MULASS,,) \
	CURSOR(Op_DIVASS,,) \
	CURSOR(Op_MODASS,,) \
	CURSOR(Op_CALL,,) \
	CURSOR(Op_SUBSCRIPT,,) \

typedef enum {
	#define CURSOR(a,b,c) Cursor_##a b c,
	CURSOR_LIST
	#undef CURSOR
} CodeCursor;

typedef uint64 CodeCursorPrimitive;

struct AstNode;

String GetCodeCursorString(CodeCursor t);
String GetOpCodeString(CodeCursor t);
bool IsTypedNode(CodeCursor src);
bool IsMetaTypedNode(CodeCursor src);
bool IsRvalReturn(CodeCursor src);


typedef enum {
	STMTP_FOR_DECL,
	STMTP_WHILE_COND,
	STMTP_FOR_POST,
	STMTP_FOR_COLLECTION,
} StmtParamType;

inline String GetStmtParamTypeString(StmtParamType t) {
	switch (t) {
		case STMTP_FOR_DECL: return "for-decl";
		case STMTP_WHILE_COND: return "while-cond";
		case STMTP_FOR_POST: return "for-post";
		case STMTP_FOR_COLLECTION: return "for-collection";
	}
	return String();
}


String GetOpCodeString(CodeCursor t);

enum {
	COOKIE_NULL,
	COOKIE_IF,
};



#endif
