#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

bool IS_DECLARED = true ;
bool IS_booleanRecognized = false; 
/*Tiny program*/
//---------------------------------
/*
int first;
int second;
int result;

first := 5;
second:= 6;

result:= first+ second;

x := 2.15;
y := 3.64;

z := x + y ;

write result;
write z;

first := 5;
second := 6;

result := first * second;

x := 2.15;
y := 3.64;

z := x * y ;

write result;
write z;  

first := 5;
second := 6;

result := first & second;

x := 2.15;
y := 3.64;

z := x & y ;

write result;
write z;


bool value;

value := true;

repeat if first <20  then first := first + 1 ; write first end ;
if first = 20 then value := false end;
until value = false ;


*/


//-----------------------------------------
// sequence of statements separated by ;
// no procedures - no declarations
// all variables are integers
// variables are declared simply by assigning values to them :=
// if-statement: if (boolean) then [else] end
// repeat-statement: repeat until (boolean)
// boolean only in if and repeat conditions < = and two mathematical expressions
// math expressions integers only, + - * / ^
// I/O read write
// Comments {}

////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char* a, const char* b)
{
	return strcmp(a, b) == 0;
}

bool StartsWith(const char* a, const char* b)
{
	int nb = strlen(b);
	return strncmp(a, b, nb) == 0;
}

void Copy(char* a, const char* b, int n = 0)
{
	if (n>0) { strncpy(a, b, n); a[n] = 0; }
	else strcpy(a, b);
}

void AllocateAndCopy(char** a, const char* b)
{
	if (b == 0) { *a = 0; return; }
	int n = strlen(b);
	*a = new char[n + 1];
	strcpy(*a, b);
}

int Power(int a, int b)
{
	if (a == 0) return 0;
	if (b == 0) return 1;
	if (b >= 1) return a*Power(a, b - 1);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile
{
	FILE* file;
	int cur_line_num;

	char line_buf[MAX_LINE_LENGTH];
	int cur_ind, cur_line_size;

	InFile(const char* str) { file = 0; if (str) file = fopen(str, "r"); cur_line_size = 0; cur_ind = 0; cur_line_num = 0; }
	~InFile() { if (file) fclose(file); }

	void SkipSpaces()
	{
		while (cur_ind<cur_line_size)
		{
			char ch = line_buf[cur_ind];
			if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
			cur_ind++;
		}
	}

	bool SkipUpto(const char* str)
	{
		while (true)
		{
			SkipSpaces();
			while (cur_ind >= cur_line_size) { if (!GetNewLine()) return false; SkipSpaces(); }

			if (StartsWith(&line_buf[cur_ind], str))
			{
				cur_ind += strlen(str);
				return true;
			}
			cur_ind++;
		}
		return false;
	}

	bool GetNewLine()
	{
		cur_ind = 0; line_buf[0] = 0;
		if (!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
		cur_line_size = strlen(line_buf);
		if (cur_line_size == 0) return false; // End of file
		cur_line_num++;
		return true;
	}

	char* GetNextTokenStr()
	{
		SkipSpaces();
		while (cur_ind >= cur_line_size) { if (!GetNewLine()) return 0; SkipSpaces(); }
		return &line_buf[cur_ind];
	}

	void Advance(int num)
	{
		cur_ind += num;
	}
};

struct OutFile
{
	FILE* file;
	OutFile(const char* str) { file = 0; if (str) file = fopen(str, "w"); }
	~OutFile() { if (file) fclose(file); }

	void Out(const char* s)
	{
		fprintf(file, "%s\n", s); fflush(file);
	}
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo
{
	InFile in_file;
	OutFile out_file;
	OutFile debug_file;

	CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
		: in_file(in_str), out_file(out_str), debug_file(debug_str)
	{
	}
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40
// Added token type ABSOLUTE_DIFFERENCE to the token types
// Added the INT,Real,Bool tokens
// Added For , From , to , inc ,startfor ,endfor tokens >> amr

enum TokenType {
	IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
	ASSIGN, EQUAL, LESS_THAN, TRUE, FALSE, For,From ,to ,inc ,startfor ,endfor, BREAK ,
	PLUS, MINUS, ABSOLUTE_DIFFERENCE, TIMES, DIVIDE, POWER,
	SEMI_COLON,
	LEFT_PAREN, RIGHT_PAREN,
	LEFT_BRACE, RIGHT_BRACE,
	ID, INT, _REAL, _BOOL,
	ENDFILE, ERROR, DECLARE
};

// Used for debugging only /////////////////////////////////////////////////////////
// Added token type AbsoluteDifference to the token types strings
// added the int,real,bool tokens for debuging
// Added For , From , to , inc ,startfor ,endfor token type string >> amr
const char* TokenTypeStr[] =
{
	"If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
	"Assign", "Equal", "LessThan","true" ,"false" , "For","From","to" ,"inc" ,"startfor ","endfor","BREAK"
	"Plus", "Minus", "AbsoluteDifference", "Times", "Divide", "Power",
	"SemiColon",
	"LeftParen", "RightParen",
	"LeftBrace", "RightBrace",
	"ID", "int","real",
	"EndFile", "Error","Declare"
};

struct Token
{
	TokenType type;
	char str[MAX_TOKEN_LEN + 1];

	Token() { str[0] = 0; type = ERROR; }
	Token(TokenType _type, const char* _str) { type = _type; Copy(str, _str); }
};

//added the int,real,bool ,TRUE ,FALSE tokens
//added reserved for the for loop . >>> amr 

const Token reserved_words[] =
{
	Token(TRUE,"true"),
	Token(FALSE, "false"),
	Token(IF, "if"),
	Token(THEN, "then"),
	Token(ELSE, "else"),
	Token(END, "end"),
	Token(REPEAT, "repeat"),
	Token(UNTIL, "until"),
	Token(READ, "read"),
	Token(WRITE, "write"),
	Token(INT,"int"),
	Token(_REAL,"real"),
	Token(For,"for"),
	Token(From,"from"),
	Token(to,"to"),
	Token(inc,"inc"),
	Token(startfor,"startfor"),
	Token(endfor,"endfor"),
	Token(BREAK,"break"),


};
const int num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment

// Added the token symbol &
const Token symbolic_tokens[] =
{
	Token(ASSIGN, ":="),
	Token(EQUAL, "="),
	Token(LESS_THAN, "<"),
	Token(PLUS, "+"),
	Token(MINUS, "-"),
	Token(ABSOLUTE_DIFFERENCE,"&"),
	Token(TIMES, "*"),
	Token(DIVIDE, "/"),
	Token(POWER, "^"),
	Token(SEMI_COLON, ";"),
	Token(LEFT_PAREN, "("),
	Token(RIGHT_PAREN, ")"),
	Token(LEFT_BRACE, "{"),
	Token(RIGHT_BRACE, "}")
};
const int num_symbolic_tokens = sizeof(symbolic_tokens) / sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch) { return (ch >= '0' && ch <= '9'); }
inline bool IsLetter(char ch) { return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')); }
inline bool IsLetterOrUnderscore(char ch) { return (IsLetter(ch) || ch == '_'); }
inline bool IsDecimalPoint(char ch) { return (ch == '.'); }

bool IsRealNumber(char* str)
{
	for (int i = 1; i < MAX_TOKEN_LEN + 1; i++)
	{
		if (IsDecimalPoint(str[i]))
		{
			return true;
		}
	}
	return false;
}
//Done added code to detect the new added tokens 
void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
	ptoken->type = ERROR;
	ptoken->str[0] = 0;

	int i;
	char* s = pci->in_file.GetNextTokenStr();
	if (!s)
	{
		ptoken->type = ENDFILE;
		ptoken->str[0] = 0;
		return;
	}

	for (i = 0; i<num_symbolic_tokens; i++)
	{
		if (StartsWith(s, symbolic_tokens[i].str))
			break;
	}

	if (i<num_symbolic_tokens)
	{
		if (symbolic_tokens[i].type == LEFT_BRACE)
		{
			pci->in_file.Advance(strlen(symbolic_tokens[i].str));
			if (!pci->in_file.SkipUpto(symbolic_tokens[i + 1].str)) return;
			return GetNextToken(pci, ptoken);
		}
		ptoken->type = symbolic_tokens[i].type;
		Copy(ptoken->str, symbolic_tokens[i].str);
	}
	// Done detects the Int,Real Token
	else if (IsDigit(s[0]))
	{
		int j = 1;
		while (IsDigit(s[j]) || IsDecimalPoint(s[j])) j++;
		if (IsRealNumber(s))  // assigning type for Number token 
		{
			ptoken->type = _REAL;
		}
		else
		{
			ptoken->type = INT;
		}
		Copy(ptoken->str, s, j);
	}
	else if (IsLetterOrUnderscore(s[0]))
	{
		int j = 1;
		while (IsLetterOrUnderscore(s[j])) j++;

		ptoken->type = ID;
		Copy(ptoken->str, s, j);

		for (i = 0; i<num_reserved_words; i++)
		{
			if (Equals(ptoken->str, reserved_words[i].str))
			{
				ptoken->type = reserved_words[i].type;
				break;
			}
		}
	}

	int len = strlen(ptoken->str);
	if (len>0) pci->in_file.Advance(len);
}

// Added the absexpr for the grammer parser

////////////////////////////////////////////////////////////////////////////////////
// Parser //////////////////////////////////////////////////////////////////////////

// program -> stmtseq
// stmtseq -> stmt { ; stmt }
// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt | declarationstmt | breakstmt
//fix
// declarationstmt -> (int|real) identifier
// Forstmt-> for <identifier> from <mathexpr> to <mathexpr> inc <mathexpr> startfor <stmtseq> endfor
// breakstmt -> break
// ifstmt -> if expr then stmtseq [ else stmtseq ] end
// repeatstmt -> repeat stmtseq until expr
// assignstmt -> identifier := expr
// readstmt -> read identifier
// writestmt -> write expr
// expr -> mathexpr [ (<|=) mathexpr ]
// mathexpr -> absexpr { (+|-) absexpr }    left associative
// absexpr -> term { & term }   left associative
// term -> factor { (*|/) factor }    left associative
// factor -> newexpr { ^ newexpr }    right associative
// newexpr -> ( mathexpr ) | number | identifier

//added INT , REAL , BOOL nodes for identifiers and values (ex. 3 , 3.5 ,true , false)
enum NodeKind {
	IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE, FOR_NODE, BREAK_NODE,
	OPER_NODE, NUM_NODE, ID_NODE, REAL_NODE, INT_NODE, BOOL_NODE, DECLARE_NODE
};
// Used for debugging only /////////////////////////////////////////////////////////


//added INT , REAL , BOOL node strings for debuging
const char* NodeKindStr[] =
{
	"If", "Repeat", "Assign", "Read", "Write","For","Break",
	"Oper", "Num", "ID","Real","Int" ,"Bool","Declaration"
};

enum ExprDataType { VOID, INTEGER, REAL, BOOLEAN };

// Used for debugging only /////////////////////////////////////////////////////////
const char* ExprDataTypeStr[] =
{
	"Void", "Integer", "Real", "Boolean"
};

#define MAX_CHILDREN 4
// done added realnum case it's REAL_NODE and bool numbool CASE it's BOOL_NODE
struct TreeNode
{
	TreeNode* child[MAX_CHILDREN];
	TreeNode* sibling; // used for sibling statements only

	NodeKind node_kind;

	union { TokenType oper; int num; double real_num; char* id; bool numbool; }; // defined for expression/int/identifier only
	ExprDataType expr_data_type; // defined for expression/int/identifier only

	int line_num;

	TreeNode() { int i; for (i = 0; i<MAX_CHILDREN; i++) child[i] = 0; sibling = 0; expr_data_type = VOID; }
};

struct ParseInfo
{
	Token next_token;
};

void Match(CompilerInfo* pci, ParseInfo* ppi, TokenType expected_token_type)
{
	pci->debug_file.Out("Start Match");

	if (ppi->next_token.type != expected_token_type) throw 0;
	
	GetNextToken(pci, &ppi->next_token);

	fprintf(pci->debug_file.file, "[%d] %s (%s)\n", pci->in_file.cur_line_num, ppi->next_token.str, TokenTypeStr[ppi->next_token.type]); fflush(pci->debug_file.file);
}

TreeNode* Expr(CompilerInfo*, ParseInfo*);

// newexpr -> ( mathexpr ) | number | identifier
//Done added code for handling different datatypes
TreeNode* NewExpr(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start NewExpr");

	// Compare the next token with the First() of possible statements
	if (ppi->next_token.type == INT || ppi->next_token.type == _REAL)
	{
		TreeNode* tree = new TreeNode;
		if (IsRealNumber(ppi->next_token.str))
		{
			tree->node_kind = REAL_NODE;
			char* num_str = ppi->next_token.str;
			int index = 0;
			int len = 0;
			for (int i = 0; i < MAX_TOKEN_LEN; i++) {
				if (ppi->next_token.str[i] == '\0') {
					break;
				}
				len++;
			}

			tree->real_num = 0;
			while ((*num_str) && (*num_str != '.')) {
				tree->real_num = tree->real_num * 10 + ((*num_str++) - '0');
				index++;
			}
			num_str++;
			int count = 1;
			while (index < len - 1) {
				double newvalue = (((double)(*num_str++) - '0') / Power(10, count++));
				tree->real_num = tree->real_num + newvalue;
				index++;
			}
		}

		else
		{
			tree->node_kind = NUM_NODE;
			char* num_str = ppi->next_token.str;
			tree->num = 0; while (*num_str) tree->num = tree->num * 10 + ((*num_str++) - '0');
		}

		tree->line_num = pci->in_file.cur_line_num;
		Match(pci, ppi, ppi->next_token.type);

		pci->debug_file.Out("End NewExpr");
		return tree;
	}

	if (ppi->next_token.type == ID)
	{
		TreeNode* tree = new TreeNode;
		tree->node_kind = ID_NODE;
		AllocateAndCopy(&tree->id, ppi->next_token.str);
		tree->line_num = pci->in_file.cur_line_num;
		Match(pci, ppi, ppi->next_token.type);

		pci->debug_file.Out("End NewExpr");
		return tree;
	}
	// assigning values for true and false tokens 
	if (ppi->next_token.type == TRUE) { 
		TreeNode* tree = new TreeNode;
		tree->node_kind = BOOL_NODE;
		tree->expr_data_type = BOOLEAN;
		tree->numbool = true;
		tree->line_num = pci->in_file.cur_line_num;
		Match(pci, ppi, ppi->next_token.type);
		return tree;
	}
	else if (ppi->next_token.type == FALSE) {
		TreeNode* tree = new TreeNode;
		tree->node_kind = BOOL_NODE;
		tree->expr_data_type = BOOLEAN;
		tree->numbool = false;
		tree->line_num = pci->in_file.cur_line_num;
		Match(pci, ppi, ppi->next_token.type);
		return tree;
	}
	if (ppi->next_token.type == LEFT_PAREN)
	{
		Match(pci, ppi, LEFT_PAREN);
		TreeNode* tree = Expr(pci, ppi);
		Match(pci, ppi, RIGHT_PAREN);

		pci->debug_file.Out("End NewExpr");
		return tree;
	}

	throw 0;
	return 0;
}

// factor -> newexpr { ^ newexpr }    right associative
TreeNode* Factor(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start Factor");

	TreeNode* tree = NewExpr(pci, ppi);

	if (ppi->next_token.type == POWER)
	{
		TreeNode* new_tree = new TreeNode;
		new_tree->node_kind = OPER_NODE;
		new_tree->oper = ppi->next_token.type;
		new_tree->line_num = pci->in_file.cur_line_num;

		new_tree->child[0] = tree;
		Match(pci, ppi, ppi->next_token.type);
		new_tree->child[1] = Factor(pci, ppi);

		pci->debug_file.Out("End Factor");
		return new_tree;
	}
	pci->debug_file.Out("End Factor");
	return tree;
}
// for <identifier> from <mathexpr> to <mathexpr> inc <mathexpr> startfor <stmtseq> endfor

// term -> factor { (*|/) factor }    left associative
TreeNode* Term(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start Term");

	TreeNode* tree = Factor(pci, ppi);

	while (ppi->next_token.type == TIMES || ppi->next_token.type == DIVIDE)
	{
		TreeNode* new_tree = new TreeNode;
		new_tree->node_kind = OPER_NODE;
		new_tree->oper = ppi->next_token.type;
		new_tree->line_num = pci->in_file.cur_line_num;

		new_tree->child[0] = tree;
		Match(pci, ppi, ppi->next_token.type);
		new_tree->child[1] = Factor(pci, ppi);

		tree = new_tree;
	}
	pci->debug_file.Out("End Term");
	return tree;
}

// Added the node AbsExpr

// absexpr -> term { & term }    left associative
TreeNode* AbsExpr(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start AbsExpr");

	TreeNode* tree = Term(pci, ppi);

	while (ppi->next_token.type == ABSOLUTE_DIFFERENCE)
	{
		TreeNode* new_tree = new TreeNode;
		new_tree->node_kind = OPER_NODE;
		new_tree->oper = ppi->next_token.type;
		new_tree->line_num = pci->in_file.cur_line_num;

		new_tree->child[0] = tree;
		Match(pci, ppi, ppi->next_token.type);
		new_tree->child[1] = Term(pci, ppi);

		tree = new_tree;
	}
	pci->debug_file.Out("End AbsExpr");
	return tree;
}

// mathexpr -> absexpr { (+|-) absexpr }    left associative
TreeNode* MathExpr(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start MathExpr");

	TreeNode* tree = AbsExpr(pci, ppi);

	while (ppi->next_token.type == PLUS || ppi->next_token.type == MINUS)
	{
		TreeNode* new_tree = new TreeNode;
		new_tree->node_kind = OPER_NODE;
		new_tree->oper = ppi->next_token.type;
		new_tree->line_num = pci->in_file.cur_line_num;

		new_tree->child[0] = tree;
		Match(pci, ppi, ppi->next_token.type);
		new_tree->child[1] = AbsExpr(pci, ppi);

		tree = new_tree;
	}
	pci->debug_file.Out("End MathExpr");
	return tree;
}

// expr -> mathexpr [ (<|=) mathexpr ]
TreeNode* Expr(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start Expr");

	TreeNode* tree = MathExpr(pci, ppi);

	if (ppi->next_token.type == EQUAL || ppi->next_token.type == LESS_THAN)
	{
		TreeNode* new_tree = new TreeNode;
		new_tree->node_kind = OPER_NODE;
		new_tree->oper = ppi->next_token.type;
		new_tree->line_num = pci->in_file.cur_line_num;

		new_tree->child[0] = tree;
		Match(pci, ppi, ppi->next_token.type);
		new_tree->child[1] = MathExpr(pci, ppi);

		pci->debug_file.Out("End Expr");
		return new_tree;
	}
	pci->debug_file.Out("End Expr");
	return tree;
}

// writestmt -> write expr
TreeNode* WriteStmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start WriteStmt");

	TreeNode* tree = new TreeNode;
	tree->node_kind = WRITE_NODE;
	tree->line_num = pci->in_file.cur_line_num;

	Match(pci, ppi, WRITE);
	tree->child[0] = Expr(pci, ppi);

	pci->debug_file.Out("End WriteStmt");
	return tree;
}

// readstmt -> read identifier
TreeNode* ReadStmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start ReadStmt");

	TreeNode* tree = new TreeNode;
	tree->node_kind = READ_NODE;
	tree->line_num = pci->in_file.cur_line_num;

	Match(pci, ppi, READ);
	if (ppi->next_token.type == ID) AllocateAndCopy(&tree->id, ppi->next_token.str);
	Match(pci, ppi, ID);

	pci->debug_file.Out("End ReadStmt");
	return tree;
}

// assignstmt -> identifier := expr
TreeNode* AssignStmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start AssignStmt");

	TreeNode* tree = new TreeNode;
	tree->node_kind = ASSIGN_NODE;
	tree->line_num = pci->in_file.cur_line_num;

	if (ppi->next_token.type == ID) AllocateAndCopy(&tree->id, ppi->next_token.str);
	Match(pci, ppi, ID);
	Match(pci, ppi, ASSIGN);
	tree->child[0] = Expr(pci, ppi);

	pci->debug_file.Out("End AssignStmt");
	return tree;
}

TreeNode* StmtSeq(CompilerInfo*, ParseInfo*);

// repeatstmt -> repeat stmtseq until expr
TreeNode* RepeatStmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start RepeatStmt");

	TreeNode* tree = new TreeNode;
	tree->node_kind = REPEAT_NODE;
	tree->line_num = pci->in_file.cur_line_num;

	Match(pci, ppi, REPEAT); tree->child[0] = StmtSeq(pci, ppi);
	Match(pci, ppi, UNTIL); tree->child[1] = Expr(pci, ppi);

	pci->debug_file.Out("End RepeatStmt");
	return tree;
}

// ifstmt -> if exp then stmtseq [ else stmtseq ] end
TreeNode* IfStmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start IfStmt");

	TreeNode* tree = new TreeNode;
	tree->node_kind = IF_NODE;
	tree->line_num = pci->in_file.cur_line_num;

	Match(pci, ppi, IF); tree->child[0] = Expr(pci, ppi);
	Match(pci, ppi, THEN); tree->child[1] = StmtSeq(pci, ppi);
	if (ppi->next_token.type == ELSE) { Match(pci, ppi, ELSE); tree->child[2] = StmtSeq(pci, ppi); }
	Match(pci, ppi, END);

	pci->debug_file.Out("End IfStmt");
	return tree;
}

// declarationstmt -> (int|real) identifier
//Done added code for handling different datatypes
TreeNode* DeclareStmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start declarationstmt");
	TreeNode* tree = new TreeNode;
	tree->node_kind = DECLARE_NODE;
	tree->line_num = pci->in_file.cur_line_num;
	// assigning and checking type for real and int and bool variables 
	if (ppi->next_token.type == _REAL) 
	{
		tree->expr_data_type = REAL;
		Match(pci, ppi, _REAL);
		//tree->node_kind = REAL_NODE;
	}
	else if (ppi->next_token.type == INT)
	{
		tree->expr_data_type = INTEGER;
		Match(pci, ppi, INT);
		//tree->node_kind = INT_NODE;
	}
	else if (ppi->next_token.type == _BOOL) {
		tree->expr_data_type = BOOLEAN;
		Match(pci, ppi, _BOOL);
	}

	if (ppi->next_token.type == ID) AllocateAndCopy(&tree->id, ppi->next_token.str);
	Match(pci, ppi, ID);


	pci->debug_file.Out("End declarationstmt");
	return tree;
}

//BreakStmt
TreeNode * BreakStmt(CompilerInfo* pci, ParseInfo* ppi) {
	TreeNode* tree = new TreeNode;
	tree->line_num = pci->in_file.cur_line_num;
	tree->node_kind = BREAK_NODE;
	Match(pci, ppi, BREAK);

	return tree;
}
//<forstmt> -> for <identifier> from <mathexpr> to <mathexpr> inc <mathexpr> startfor <stmtseq> endfor
TreeNode* Forstmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start Factor");

	TreeNode* treenode = new TreeNode;

	if (ppi->next_token.type == For)
	{
		treenode->line_num = pci->in_file.cur_line_num;
		Match(pci, ppi, For);
		treenode->node_kind = FOR_NODE;

		if (ppi->next_token.type == ID) AllocateAndCopy(&treenode->id, ppi->next_token.str);
		Match(pci, ppi, ID);
		Match(pci, ppi, From);

		treenode->child[0] = MathExpr(pci, ppi);
		Match(pci, ppi, to);
		treenode->child[1] = MathExpr(pci, ppi);

		Match(pci, ppi, inc);
		treenode->child[2] = MathExpr(pci, ppi);
		Match(pci, ppi, startfor);
		treenode->child[3] = StmtSeq(pci, ppi);

		return treenode;
	}



	pci->debug_file.Out("End Factor");
	return treenode;
}
// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt | declarationstmt | Forstmt
TreeNode* Stmt(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start Stmt");

	// Compare the next token with the First() of possible statements
	TreeNode* tree = 0;
	if (ppi->next_token.type == IF) tree = IfStmt(pci, ppi);
	else if (ppi->next_token.type == REPEAT) tree = RepeatStmt(pci, ppi);
	else if (ppi->next_token.type == ID) tree = AssignStmt(pci, ppi);
	else if (ppi->next_token.type == READ) tree = ReadStmt(pci, ppi);
	else if (ppi->next_token.type == WRITE) tree = WriteStmt(pci, ppi);
	else if (ppi->next_token.type == For) tree = Forstmt(pci, ppi);
	else if (ppi->next_token.type == BREAK) tree = BreakStmt(pci, ppi);
	//Done added a condition for handling Int,Real,Bool tokens
	else if (ppi->next_token.type == _REAL || ppi->next_token.type == INT || ppi->next_token.type == _BOOL) 
		tree = DeclareStmt(pci, ppi);

	pci->debug_file.Out("End Stmt");
	return tree;
}

// stmtseq -> stmt { ; stmt }
TreeNode* StmtSeq(CompilerInfo* pci, ParseInfo* ppi)
{
	pci->debug_file.Out("Start StmtSeq");

	TreeNode* first_tree = Stmt(pci, ppi);
	TreeNode* last_tree = first_tree;

	// If we did not reach one of the Follow() of StmtSeq(), we are not done yet
	//
	while (ppi->next_token.type != ENDFILE && ppi->next_token.type != END &&
		ppi->next_token.type != ELSE && ppi->next_token.type != UNTIL)
	{
			Match(pci, ppi, SEMI_COLON);
			if (ppi->next_token.type == BREAK)
			{
						
				TreeNode* next_tree = Stmt(pci, ppi);
				last_tree->sibling = next_tree;
				last_tree = next_tree;

				Match(pci, ppi, SEMI_COLON);
				while (ppi->next_token.type != ENDFILE && ppi->next_token.type != END &&
					ppi->next_token.type != ELSE && ppi->next_token.type != UNTIL && ppi->next_token.type != endfor)
				{
					Match(pci, ppi, ppi->next_token.type);/* loop to skip until endfor */
				}
			}
			if (ppi->next_token.type == endfor)
			{
				Match(pci, ppi, endfor);
				Match(pci, ppi, SEMI_COLON);
			}
			TreeNode* next_tree = Stmt(pci, ppi);
			last_tree->sibling = next_tree;
			last_tree = next_tree;
	}

	pci->debug_file.Out("End StmtSeq");
	return first_tree;
}

// program -> stmtseq
TreeNode* Parse(CompilerInfo* pci)
{
	ParseInfo parse_info;
	GetNextToken(pci, &parse_info.next_token);

	TreeNode* syntax_tree = StmtSeq(pci, &parse_info);

	if (parse_info.next_token.type != ENDFILE)
		pci->debug_file.Out("Error code ends before file ends");

	return syntax_tree;
}
//Done added configuration for printing out the different datatypes values
void PrintTree(TreeNode* node, int sh = 0)
{
	int i, NSH = 3;
	for (i = 0; i<sh; i++) printf(" ");

	printf("[%s]", NodeKindStr[node->node_kind]);

	if (node->node_kind == OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
	else if (node->node_kind == NUM_NODE) 
		printf("[%d]", node->num);
	else if (node->node_kind == REAL_NODE) printf("[%f]", node->real_num);
	else if (node->node_kind == ID_NODE || node->node_kind == READ_NODE || node->node_kind == ASSIGN_NODE) printf("[%s]", node->id);
	
	if (node->expr_data_type != VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);

	printf("\n");

	for (i = 0; i<MAX_CHILDREN; i++) if (node->child[i]) PrintTree(node->child[i], sh + NSH);
	if (node->sibling) PrintTree(node->sibling, sh);
}

void DestroyTree(TreeNode* node)
{
	int i;

	if (node->node_kind == ID_NODE || node->node_kind == READ_NODE || node->node_kind == ASSIGN_NODE)
		if (node->id) delete[] node->id;

	for (i = 0; i<MAX_CHILDREN; i++) if (node->child[i]) DestroyTree(node->child[i]);
	if (node->sibling) DestroyTree(node->sibling);

	delete node;
}

// report an exception
void abort(const char* msg) {
	// print error message 
	// then exits
	printf("Error recieved !![Analyze Funcion] : %s", msg);
	abort();
}

////////////////////////////////////////////////////////////////////////////////////
// Analyzer ////////////////////////////////////////////////////////////////////////

const int SYMBOL_HASH_SIZE = 10007;

struct LineLocation
{
	int line_num;
	LineLocation* next;
};

struct VariableInfo
{
	char* name;
	int memloc;
	ExprDataType datatype;
	LineLocation* head_line; // the head of linked list of source line locations
	LineLocation* tail_line; // the tail of linked list of source line locations
	VariableInfo* next_var; // the next variable in the linked list in the same hash bucket of the symbol table
};

struct SymbolTable
{
	int num_vars;
	int num_bool; //added another counter for boolean variables 
	VariableInfo* var_info[SYMBOL_HASH_SIZE];

	SymbolTable() { num_vars = 0; num_bool = 0; int i; for (i = 0; i < SYMBOL_HASH_SIZE; i++) var_info[i] = 0; }

	int Hash(const char* name)
	{
		int i, len = strlen(name);
		int hash_val = 11;
		for (i = 0; i<len; i++) hash_val = (hash_val * 17 + (int)name[i]) % SYMBOL_HASH_SIZE;
		return hash_val;
	}

	VariableInfo* Find(const char* name)
	{
		int h = Hash(name);
		VariableInfo* cur = var_info[h];
		while (cur)
		{
			if (Equals(name, cur->name)) return cur;
			cur = cur->next_var;
		}
		return 0;
	}

	void Insert(const char *name, int line_num, ExprDataType datatype)
	{
		LineLocation* lineloc = new LineLocation;
		lineloc->line_num = line_num;
		lineloc->next = 0;

		int h = Hash(name);
		VariableInfo* prev = 0;
		VariableInfo* cur = var_info[h];

		while (cur)
		{
			if (Equals(name, cur->name))
			{
				// just add this line location to the list of line locations of the existing var
				cur->tail_line->next = lineloc;
				cur->tail_line = lineloc;
				return;
			}
			prev = cur;
			cur = cur->next_var;
		}

		VariableInfo* vi = new VariableInfo;
		vi->head_line = vi->tail_line = lineloc;
		vi->next_var = 0;
		
		//adding variable location
		if (datatype == BOOLEAN) {
			vi->memloc = num_bool++;
		}
		else {
			vi->memloc = num_vars++;
		}
		AllocateAndCopy(&vi->name, name);
		vi->datatype = datatype;

		if (!prev) var_info[h] = vi;
		else prev->next_var = vi;
	}

	void Print()
	{
		int i;
		for (i = 0; i<SYMBOL_HASH_SIZE; i++)
		{
			VariableInfo* curv = var_info[i];
			while (curv)
			{
				printf("[Var=%s][Mem=%d]", curv->name, curv->memloc);
				LineLocation* curl = curv->head_line;
				while (curl)
				{
					printf("[Line=%d]", curl->line_num);
					curl = curl->next;
				}
				printf("\n");
				curv = curv->next_var;
			}
		}
	}

	void Destroy()
	{
		int i;
		for (i = 0; i<SYMBOL_HASH_SIZE; i++)
		{
			VariableInfo* curv = var_info[i];
			while (curv)
			{
				LineLocation* curl = curv->head_line;
				while (curl)
				{
					LineLocation* pl = curl;
					curl = curl->next;
					delete pl;
				}
				VariableInfo* p = curv;
				curv = curv->next_var;
				delete p;
			}
			var_info[i] = 0;
		}
	}
};
// handle type checking for expressions and throwing exceptions if an type error occurs 
void Analyze(TreeNode* node, SymbolTable* symbol_table)
{
	int i;
	bool is_visited = false ;
	if (node->node_kind == DECLARE_NODE) {
		is_visited = true;
		if (IS_DECLARED == false) {
			throw "Error !! Can't initialize variables after declaration";
		}
		if (symbol_table->Find(node->id) != 0) {
			throw "Error !! variable already declared";
		}
		symbol_table->Insert(node->id, node->line_num, node->expr_data_type);
	}
	

	for (i = 0; i<MAX_CHILDREN; i++) if (node->child[i]) Analyze(node->child[i], symbol_table);
	
	
	if (node->node_kind == FOR_NODE)
	{
		// to exclude child stmtsequence
		if (symbol_table->Find(node->id)->datatype != INTEGER)
		{
			throw "Error !! Identifier in For loop must be of type Integer";

		}	
		for (int i = 0; i < MAX_CHILDREN - 1; i++)
		{	
			if (node->child[i]->expr_data_type != INTEGER)
			{
				throw "Error !! For Loop variables must be of Type Integer";
			}
		}
	}
	

	if (node->node_kind == ASSIGN_NODE) {
		if (!symbol_table->Find(node->id))
			throw "variable specified not Declared [check Declaration]";
		node->expr_data_type = symbol_table->Find(node->id)->datatype;
	}

	//Done added code for setting the node datatype
	if (node->node_kind == OPER_NODE)
	{
		if (node->oper == EQUAL || node->oper == LESS_THAN) node->expr_data_type = BOOLEAN;
		else if (node->child[0]->expr_data_type == INTEGER && node->child[1]->expr_data_type == INTEGER)
			node->expr_data_type = INTEGER;
		else if (node->child[0]->expr_data_type == REAL && node->child[1]->expr_data_type == REAL)
			node->expr_data_type = REAL;

	}

	if (node->node_kind == ID_NODE) node->expr_data_type = symbol_table->Find(node->id)->datatype;
	if (node->node_kind == NUM_NODE) node->expr_data_type = INTEGER;
	if (node->node_kind == REAL_NODE) node->expr_data_type = REAL;

	if (node->node_kind == OPER_NODE)
	{
		TreeNode* firstChild = node->child[0];
		TreeNode* secondChild = node->child[1];
		//Done added a check to not allow any kind of operation on boolean variables
		if (firstChild->expr_data_type == BOOLEAN) {
			return;
		}

		if ((firstChild->expr_data_type != INTEGER && firstChild->expr_data_type != REAL)
			|| (secondChild->expr_data_type != INTEGER && secondChild->expr_data_type != REAL)) {
			throw("ERROR Operator applied to non-numeric variable\n");
		}

		// Added to handle the error of variables of different types
		if (node->child[0]->expr_data_type != node->child[1]->expr_data_type) {
			throw ("ERROR Operator can be applied on 2 variables of same data types only\n");
		}
	}



	else if (node->node_kind == IF_NODE && node->child[0]->expr_data_type != BOOLEAN) throw ("ERROR If test must be BOOLEAN\n");
	else if (node->node_kind == REPEAT_NODE && node->child[1]->expr_data_type != BOOLEAN) 
		throw ("ERROR Repeat test must be BOOLEAN\n");
	//if (node->node_kind == WRITE_NODE && (node->child[0]->expr_data_type != INTEGER && node->child[0]->expr_data_type != REAL)) throw("ERROR Write works only for INTEGER and REAL\n");
	//if (node->node_kind == ASSIGN_NODE && (node->child[0]->expr_data_type != INTEGER && node->child[0]->expr_data_type != REAL )) throw("ERROR Assign works only for INTEGER\n");
	
	if (!is_visited)
		IS_DECLARED = false;


	if (node->sibling) Analyze(node->sibling, symbol_table);
}

////////////////////////////////////////////////////////////////////////////////////
// Code Generator //////////////////////////////////////////////////////////////////

double Power(double a, double b)
{
	if (a == 0) return 0;
	if (b == 0) return 1;
	if (b >= 1) return a*Power(a, b - 1);
	return 0;
}

double Evaluate(TreeNode* node, SymbolTable* symbol_table, double* variables)
{
	if (node->node_kind == NUM_NODE) return node->num;
	if (node->node_kind == REAL_NODE) return node->real_num;
	if (node->node_kind == ID_NODE) return variables[symbol_table->Find(node->id)->memloc];

	double a = Evaluate(node->child[0], symbol_table, variables);
	double b = Evaluate(node->child[1], symbol_table, variables);

	if (node->oper == EQUAL) return a == b;
	if (node->oper == LESS_THAN) return a<b;
	if (node->oper == PLUS) return a + b;
	if (node->oper == MINUS) return a - b;
	// Added the evaluation for the expr absolute difference
	// Returns a-b if a-b is positive and returns b-a if a-b is negative
	if (node->oper == ABSOLUTE_DIFFERENCE) return (a - b > 0) ? a - b : b - a;
	if (node->oper == TIMES) return a*b;
	if (node->oper == DIVIDE) return a / b;
	if (node->oper == POWER) return Power(a, b);
	throw 0;
	return 0;
}
// added overloaded evaluate for booleans expressions and boolean variables  
bool Evaluate(TreeNode* node, SymbolTable* symbol_table, bool* variables)
{
	if (node->node_kind == ID_NODE) return variables[symbol_table->Find(node->id)->memloc];

	if (node->node_kind == BOOL_NODE) {
		return node->numbool;
	}
	//handling until condition for repeat stmt (< , > , :slight_smile:
	if (node->node_kind == OPER_NODE) {
		return variables[symbol_table->Find(node->child[0]->id)->memloc] == node->child[1]->numbool ? true : false;
	}

	else
		return 0;
}

void RunProgram(TreeNode* node, SymbolTable* symbol_table, double* variables, bool *variables_bool)
{

	if (node->node_kind == IF_NODE)
	{
		double cond = Evaluate(node->child[0], symbol_table, variables);
		int cond1 = (int)cond; //casting to integer for condition
		if (cond1) {
			RunProgram(node->child[1], symbol_table, variables, variables_bool);
			if (IS_booleanRecognized) return;
		}
		else if (node->child[2]) RunProgram(node->child[2], symbol_table, variables, variables_bool);
	}
	
	if (node->node_kind == BREAK_NODE)
	{
		IS_booleanRecognized = true;
	}

	// for <identifier> from <mathexpr> to <mathexpr> inc <mathexpr> startfor <stmtseq> endfor
	// adding Evaluation for for loop 
	if (node->node_kind == FOR_NODE)
	{
		 double x = Evaluate(node->child[0], symbol_table, variables);
		 int xx = (int)x;
		 variables[symbol_table->Find(node->id)->memloc] = xx ;

		 double x1 = Evaluate(node->child[1], symbol_table, variables);
		 double inc = Evaluate(node->child[2], symbol_table, variables);
		 int increment_v = (int)inc;
		
		 while (xx < x1)
		 {
			 RunProgram(node->child[3], symbol_table, variables, variables_bool);
			 if(IS_booleanRecognized) return;
			 xx += increment_v;
			 variables[symbol_table->Find(node->id)->memloc] = xx;
		 }          

	}

	//added code for checking on datatypes when assigning
	if (node->node_kind == ASSIGN_NODE)
	{
		TreeNode* firstChild = node->child[0];
		TreeNode* firstGrandChild = node->child[0]->child[0];
		TreeNode* secondGrandChild = node->child[0]->child[1];
		if (firstChild->node_kind == OPER_NODE)
		{
			if (firstChild->child[0]->expr_data_type == node->expr_data_type) {
				if ((firstGrandChild->expr_data_type != INTEGER && firstGrandChild->expr_data_type != REAL)
					|| (secondGrandChild->expr_data_type != INTEGER && secondGrandChild->expr_data_type != REAL))
					throw("ERROR Operator applied to non-numeric variable\n");
				// Added to handle the error of variables of different types
				if (firstGrandChild->expr_data_type != secondGrandChild->expr_data_type)
					throw ("ERROR Operator can be applied on 2 variables of same data types only\n");
			}
			else
				throw ("Parent operator MUST BE OF THE SAME TYPE\n");

			double v = Evaluate(firstChild, symbol_table, variables);
			variables[symbol_table->Find(node->id)->memloc] = v;
		}
		// handling assigning booleans 
		else {
			// handling assigning of not the same type 
			if (firstChild->expr_data_type != node->expr_data_type) {
				throw "CAN'T ASSIGN MUST BE OF THE SAME TYPE";
			}

			if (node->child[0]->node_kind == BOOL_NODE) {
				bool vl = Evaluate(node->child[0], symbol_table, variables_bool);
				variables_bool[symbol_table->Find(node->id)->memloc] = vl;
			}
			// for integers and doubles 
			else {
				double v = Evaluate(firstChild, symbol_table, variables);
				variables[symbol_table->Find(node->id)->memloc] = v;
			}
		}

	}
	if (node->node_kind == READ_NODE)
	{
		int index = symbol_table->Find(node->id)->memloc;
		printf("Enter %s: ", node->id);
		scanf("%lf", &variables[symbol_table->Find(node->id)->memloc]);
	}
	//Done added code to enable different datatypes when outputing 
	if (node->node_kind == WRITE_NODE)
	{

		if (node->child[0]->expr_data_type == BOOLEAN) {
			bool val = Evaluate(node->child[0], symbol_table, variables_bool);
			if (val)
				printf("Val: %s\n", "true");
			else
				printf("Val: %s\n", "false");

		}
		else {
			double v = Evaluate(node->child[0], symbol_table, variables);
			if (node->child[0]->expr_data_type == INTEGER)
			{
				int value = (int)v;

				printf("Val: %d\n", value);
			}
			else
				printf("Val: %3f\n", v);
		}
	}
	if (node->node_kind == REPEAT_NODE)
	{
		do
		{
			RunProgram(node->child[0], symbol_table, variables, variables_bool);
		} while (!Evaluate(node->child[1], symbol_table, variables_bool));
	}
	if (node->sibling) RunProgram(node->sibling, symbol_table, variables, variables_bool);
}
/*
* DONE
---added an array for boolean variables and made the variables array store
double to store both datatypes (Int,Real) in it
---changed the function runProgram to have recieve both variables and boolean variables arrays
*/
void RunProgram(TreeNode* syntax_tree, SymbolTable* symbol_table)
{
	int i;
	double* variables = new double[symbol_table->num_vars];
	bool* variables_bool = new bool[symbol_table->num_bool];

	for (i = 0; i<symbol_table->num_vars; i++) variables[i] = 0;
	for (i = 0; i<symbol_table->num_bool; i++) variables_bool[i] = 0;


	RunProgram(syntax_tree, symbol_table, variables, variables_bool);
	delete[] variables;
}

////////////////////////////////////////////////////////////////////////////////////
// Scanner and Compiler ////////////////////////////////////////////////////////////

// added try and catch for handling exception thrown by Anaylze function .
void StartCompiler(CompilerInfo* pci)
{
	TreeNode* syntax_tree = Parse(pci);

	SymbolTable symbol_table;

	Analyze(syntax_tree, &symbol_table);

	printf("Symbol Table:\n");
	symbol_table.Print();
	printf("---------------------------------\n"); fflush(NULL);

	printf("Syntax Tree:\n");
	PrintTree(syntax_tree);
	printf("---------------------------------\n"); fflush(NULL);

	printf("Run Program:\n");

	RunProgram(syntax_tree, &symbol_table);


	printf("---------------------------------\n"); fflush(NULL);

	symbol_table.Destroy();
	DestroyTree(syntax_tree);
}

////////////////////////////////////////////////////////////////////////////////////
// Scanner only ////////////////////////////////////////////////////////////////////

void StartScanner(CompilerInfo* pci)
{
	Token token;

	while (true)
	{
		GetNextToken(pci, &token);
		printf("[%d] %s (%s)\n", pci->in_file.cur_line_num, token.str, TokenTypeStr[token.type]); fflush(NULL);
		if (token.type == ENDFILE || token.type == ERROR) break;
	}
}

////////////////////////////////////////////////////////////////////////////////////

int main()
{
	printf("Start main()\n"); fflush(NULL);

	CompilerInfo compiler_info("input.txt", "output.txt", "debug.txt");

	// handling exceptions for Analyzer 
	try {
		StartCompiler(&compiler_info);
	}
	catch (const char* msg) {
		abort(msg);
	}

	printf("End main()\n"); fflush(NULL);

	system("pause");
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
