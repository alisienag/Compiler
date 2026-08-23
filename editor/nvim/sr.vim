if exists("b:current_syntax")
  finish
endif

syn keyword srKeyword       fn let mut as
syn keyword srConditional   if else
syn keyword srRepeat        while
syn keyword srStatement     ret break continue
syn keyword srBoolean       true false
syn keyword srType          i8 u8 i16 u16 i32 u32 i64 u64 bool void

syn match   srNumber        "\<\d\+\>"
syn region  srString        start=+"+ skip=+\\.+ end=+"+ contains=srEscape
syn region  srChar          start=+'+ skip=+\\.+ end=+'+ contains=srEscape
syn match   srEscape        contained "\\[nrt0\\'\"]"

syn match   srComment       "//.*$" contains=srTodo
syn keyword srTodo          contained TODO FIXME XXX NOTE HACK

syn match   srFuncDef       "\<fn\s\+\zs\h\w*"
syn match   srFuncCall      "\<\h\w*\ze\s*("
syn match   srParamName     "\<\h\w*\ze\s*:"

syn match   srOperator      "=>"
syn match   srOperator      "==\|!=\|<=\|>=\|&&\|||"
syn match   srOperator      "[+\-*/!<>=]"
syn match   srDelim         "[(){}\[\],;:]"

hi def link srKeyword       Keyword
hi def link srConditional   Conditional
hi def link srRepeat        Repeat
hi def link srStatement     Statement
hi def link srBoolean       Boolean
hi def link srType          Type
hi def link srNumber        Number
hi def link srString        String
hi def link srChar          Character
hi def link srEscape        SpecialChar
hi def link srComment       Comment
hi def link srTodo          Todo
hi def link srFuncDef       Function
hi def link srFuncCall      Function
hi def link srParamName     Identifier
hi def link srOperator      Operator
hi def link srDelim         Delimiter

let b:current_syntax = "sr"
