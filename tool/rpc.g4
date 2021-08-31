grammar rpc;

primitive:  kind=PRIMITIVE;
collection: '[' WS* elementType=type WS* ']';
aggregate:  '{' members=varList '}';
type: p=primitive | a=aggregate | c=collection | n=IDENTIFIER;
var: WS* name=IDENTIFIER VALSEP t=type WS*;
varList: WS* vars+=var? (LIST_SEPARATOR vars+=var)* WS* ;
action: name=IDENTIFIER WS* '(' args=varList ')';
function: call=action (VALSEP ret=type)?;

typeAlias: name=IDENTIFIER NAMEVALSEP value=type;

fwdCall: '!' WS* sym = action;
callBack: '@' WS* sym = action;
sessionItem: fwd=fwdCall | bwd=callBack | ctr=function;
session: name=IDENTIFIER WS* '<' WS* items+=sessionItem (DECLSEP+ (items+=sessionItem)? WS*)*? '>';

item: WS* (func=function | alias=typeAlias | sess=session | EOF) WS*?;

rpc: items+=item (DECLSEP+ items+=item)*;
 
PRIMITIVE:      [IiUu][1248];
IDENTIFIER:     [_a-zA-Z][_a-zA-Z0-9]*;
LIST_SEPARATOR: WS* ',' WS*;
VALSEP:      	WS* ':' WS*;
DECLSEP:        WS* ';' WS*;
NAMEVALSEP:     WS* '=' WS*;
BLOCKCOMMENT:	'/*' .*? '*/' -> skip;
LINECOMMENT:	'//' ~[\r\n]* (EOF | [\r\n]+) -> skip;
WS:             ('\r'? '\n') | [\t ] | BLOCKCOMMENT | LINECOMMENT ;