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
	CURSOR(Expr,,) \
	CURSOR(Literal,,) \
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
	CURSOR(MetaFunction,,)			/* MetaStaticFunction /*| MetaFunction_METHOD | MetaFunction_BUILTIN,,) */ \
	CURSOR(ClassPath_MetaParam,,)	/* MetaParameter | NamePart,,) */ \
	CURSOR(ClassPath_MetaVar,,)		/* MetaVariable | NamePart,,) */ \
	\
	CURSOR(MetaDecl,,)				/* NamePart | MetaValueDecl | MetaTypeDecl | MetaFunction | MetaRval | MetaCtor | MetaResolve,,) */ \
	CURSOR(ClassPath_MetaDecl,,)	/* ClassPath_MetaParam | ClassPath_MetaVar | MetaFunction | MetaClass,,) */ \
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

typedef enum {
	#define CURSOR(a,b,c) Cursor_##a b c,
	CURSOR_LIST
	#undef CURSOR
} CodeCursor;

typedef uint64 CodeCursorPrimitive;

struct AstNode;

String GetCodeCursorString(CodeCursor t);
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


typedef enum {
	OP_NULL,
	OP_INC,
	OP_DEC,
	OP_POSTINC,
	OP_POSTDEC,
	OP_NEGATIVE,
	OP_POSITIVE,
	OP_NOT,
	OP_NEGATE,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_LSH,
	OP_RSH,
	OP_GREQ,
	OP_LSEQ,
	OP_GREATER,
	OP_LESS,
	OP_EQ,
	OP_INEQ,
	OP_BWAND,
	OP_BWXOR,
	OP_BWOR,
	OP_AND,
	OP_OR,
	OP_COND,
	OP_ASSIGN,
	OP_ADDASS,
	OP_SUBASS,
	OP_MULASS,
	OP_DIVASS,
	OP_MODASS,
	OP_CALL,
	OP_SUBSCRIPT,
	
} OpType;


inline String GetOpString(OpType t) {
	switch (t) {
		case OP_INC: return "increase";
		case OP_DEC: return "decrease";
		case OP_POSTINC: return "post-increase";
		case OP_POSTDEC: return "post-decrease";
		case OP_NEGATIVE: return "negative";
		case OP_POSITIVE: return "positive";
		case OP_NOT: return "not";
		case OP_NEGATE: return "negate";
		case OP_ADD: return "add";
		case OP_SUB: return "subtract";
		case OP_MUL: return "multiply";
		case OP_DIV: return "divide";
		case OP_MOD: return "modulus";
		case OP_LSH: return "left-shift";
		case OP_RSH: return "right-shift";
		case OP_GREQ: return "greater-or-equal";
		case OP_LSEQ: return "less-or-equal";
		case OP_GREATER: return "greater";
		case OP_LESS: return "less";
		case OP_EQ: return "equal";
		case OP_INEQ: return "inequal";
		case OP_BWAND: return "bitwise-and";
		case OP_BWXOR: return "bitwise-xor";
		case OP_BWOR: return "bitwise-or";
		case OP_AND: return "and";
		case OP_OR: return "op";
		case OP_COND: return "conditional";
		case OP_ASSIGN: return "assign";
		case OP_ADDASS: return "add-and-assign";
		case OP_SUBASS: return "subtract-and-assign";
		case OP_MULASS: return "multiply-and-assign";
		case OP_DIVASS: return "divide-and-assign";
		case OP_MODASS: return "modulus-and-assign";
		case OP_CALL: return "call";
		case OP_SUBSCRIPT: return "subscript";
		default: return "<invalid>";
	}
}

inline String GetOpCodeString(OpType t) {
	switch (t) {
		case OP_INC: return "++";
		case OP_DEC: return "--";
		case OP_POSTINC: return "++";
		case OP_POSTDEC: return "--";
		case OP_NEGATIVE: return "-";
		case OP_POSITIVE: return "+";
		case OP_NOT: return "!";
		case OP_NEGATE: return "~";
		case OP_ADD: return "+";
		case OP_SUB: return "-";
		case OP_MUL: return "*";
		case OP_DIV: return "/";
		case OP_MOD: return "%";
		case OP_LSH: return "<<";
		case OP_RSH: return ">>";
		case OP_GREQ: return ">=";
		case OP_LSEQ: return "<=";
		case OP_GREATER: return ">";
		case OP_LESS: return "<";
		case OP_EQ: return "==";
		case OP_INEQ: return "!=";
		case OP_BWAND: return "&";
		case OP_BWXOR: return "^";
		case OP_BWOR: return "|";
		case OP_AND: return "&&";
		case OP_OR: return "||";
		case OP_COND: return "?:";
		case OP_ASSIGN: return "=";
		case OP_ADDASS: return "+=";
		case OP_SUBASS: return "-=";
		case OP_MULASS: return "*=";
		case OP_DIVASS: return "/=";
		case OP_MODASS: return "%=";
		default: return "<invalid>";
	}
}


typedef enum {
	CONST_NULL,
	CONST_BOOL,
	CONST_INT32,
	CONST_INT64,
	CONST_DOUBLE,
	CONST_STRING,
} ConstType;

inline String GetConstString(ConstType t) {
	switch (t) {
		case CONST_NULL:	return "null";
		case CONST_BOOL:	return "bool";
		case CONST_INT32:	return "int32";
		case CONST_INT64:	return "int64";
		case CONST_DOUBLE:	return "double";
		case CONST_STRING:	return "string";
		default: return "invalid";
	}
}


enum {
	COOKIE_NULL,
	COOKIE_IF,
};



#endif
