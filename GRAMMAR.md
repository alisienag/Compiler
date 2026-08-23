# Grammar

LL(1) — one token of lookahead. All alternatives below have disjoint FIRST sets,
so the parser never backtracks and parsing is linear in the input length.

## Lexical

```
ident   -> [A-Za-z][A-Za-z0-9]*        ; minus the keyword set
number  -> [0-9]+
string  -> '"' char* '"'               ; one token
charlit -> "'" char "'"                ; one token, yields u8
comment -> '//' ... end-of-line        ; skipped by the lexer
```

Keywords:

```
fn  let  mut  ret  if  else  while  break  continue  as  true  false
i8  u8  i16  u16  i32  u32  i64  u64  bool  void
```

Multi-character operators, lexed by maximal munch as single tokens:

```
==  !=  <=  >=  &&  ||
```

## Program

```
program   -> function* EOF
function  -> 'fn' ident '(' params? ')' ':' type '=' '>' (block | expression ';')
params    -> param (',' param)*
param     -> 'mut'? ident ':' type
```

The `=>` before the body is always required; what follows is either a block or a
single expression. An expression body desugars to a block containing one `ret`,
so every function has the same shape after parsing.

## Statements

```
statement -> letStmt
           | retStmt
           | ifStmt
           | whileStmt
           | breakStmt
           | continueStmt
           | block
           | exprStmt

letStmt      -> ('let' | 'mut') ident (':' type)? '=' expression ';'
retStmt      -> 'ret' expression? ';'
ifStmt       -> 'if' expression block ('else' (ifStmt | block))?
whileStmt    -> 'while' expression block
breakStmt    -> 'break' ';'
continueStmt -> 'continue' ';'
block        -> '{' statement* '}'
exprStmt     -> expression ('=' expression)? ';'
```

`let` binds immutably, `mut` mutably; they are alternatives rather than
`let mut`. Omitting the type annotation infers it from the initialiser.

`exprStmt` covers both `expr;` and `lhs = expr;`. They are one rule because no
finite lookahead distinguishes `a[i][j] = 5;` from `a[i][j];` — the parser reads
an expression first, then checks for `=`. Whether the left side is assignable is
a semantic question, not a syntactic one.

Conditions need no parentheses only because no expression can begin with `{`.
Braces are mandatory, which also removes the dangling-`else` ambiguity: the
then-branch is always a block, so a trailing `else` has exactly one legal
attachment.

## Expressions

Lowest to highest precedence. Each level is one function in the parser, so the
call chain is the precedence table.

```
expression     -> logicalOr
logicalOr      -> logicalAnd ('||' logicalAnd)*
logicalAnd     -> comparison ('&&' comparison)*
comparison     -> additive (('==' | '!=' | '<' | '>' | '<=' | '>=') additive)?
additive       -> multiplicative (('+' | '-') multiplicative)*
multiplicative -> cast (('*' | '/') cast)*
cast           -> unary ('as' type)*
unary          -> '-' unary
                | '!' unary
                | postfix
postfix        -> primary ('[' expression ']' | '(' args? ')')*
primary        -> number
                | string
                | charlit
                | 'true'
                | 'false'
                | ident
                | '(' expression ')'
args           -> expression (',' expression)*
```

`comparison` uses `?` rather than `*`: comparisons are **non-associative**, so
`a < b < c` is rejected instead of parsing as `(a < b) < c`.

Casts are postfix, which avoids any collision with `<` and lets conversions chain
left to right: `x as u8 as i64`.

Indexing and calls are the two arms of a single postfix loop, so `f(x)[0]` and
`a[i][j]` both work and the two node kinds are siblings in the AST.

## Types

```
type -> 'i8' | 'u8' | 'i16' | 'u16' | 'i32' | 'u32' | 'i64' | 'u64'
      | 'bool' | 'void' | '[' type ']'
```

Array types nest: `[[i32]]`. An array-typed value is a pointer occupying one
8-byte slot; only its elements are packed at their own width.

## Semantic rules not expressible here

These are enforced by later passes rather than by the grammar:

- The left side of `=` must be a name or an index expression rooted at one, and
  that root must be declared `mut`.
- `break` and `continue` require an enclosing loop.
- A condition may be any type except `void`; a non-zero value is true. There is
  no implicit conversion to `bool`.
- Integer literals take their type from context where one exists, and default to
  `i32` otherwise.
- `void` may not be bound to a variable.
- Casts between any two non-`void` types are permitted. Narrowing truncates and
  widening extends according to the *source* type's signedness.
