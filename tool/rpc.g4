grammar rpc;

rpc: items+=item (DECLSEP+ items+=item)*?;

item: WS* (push=symbol | pull=getter | alias=typeAlias | sess=session | EOF) WS*;

session: name=IDENTIFIER WS* '<' WS* items+=sessionItem (DECLSEP+ items+=sessionItem)* WS* '>';
sessionItem: fwd=fwdCall | bwd=callBack;
fwdCall: '!' WS* sym = symbol;
callBack: '@' WS* sym = symbol;
symbol: name=IDENTIFIER WS* '(' args=varList ')';
typeAlias: name=IDENTIFIER NAMEVALSEP value=type;
getter: sym=symbol VALSEP ret=type;

varList: WS* vars+=var? (LIST_SEPARATOR vars+=var)* WS* ;
var: WS* name=IDENTIFIER VALSEP t=type WS*;
type: p=primitive | a=aggregate | c=collection | n=IDENTIFIER;

primitive:  kind=PRIMITIVE;
collection: '[' WS* elementType=type WS* ']';
aggregate:  '{' members=varList '}';
 
PRIMITIVE:      [IiUu][1248];
IDENTIFIER:     [_a-zA-Z][_a-zA-Z0-9]*;
LIST_SEPARATOR: WS* ',' WS*;
VALSEP:      	WS* ':' WS*;
DECLSEP:        WS* ';' WS*;
NAMEVALSEP:     WS* '=' WS*;
BLOCKCOMMENT:	'/*' .*? '*/' -> skip;
LINECOMMENT:	'//' ~[\r\n]* (EOF | [\r\n]+) -> skip;
WS:             ('\r'? '\n') | [\t ] | BLOCKCOMMENT | LINECOMMENT ;