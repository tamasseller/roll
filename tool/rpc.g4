grammar rpc;

primitive:  kind=PRIMITIVE;
collection: '[' WS* elementType=type WS* ']';
aggregate:  '{' members=varList '}';
type: p=primitive | a=aggregate | c=collection | n=IDENTIFIER;
var: WS* (docs=DOCS)? WS* name=IDENTIFIER VALSEP t=type  WS*;
varList: WS* vars+=var? (LISTSEP vars+=var)* WS* ;
action: name=IDENTIFIER WS* '(' args=varList ')';
function: call=action (VALSEP ret=type)?;

typeAlias: name=IDENTIFIER NAMEVALSEP value=type;

fwdCall: '!' WS* sym=action;
callBack: '@' WS* sym=action;
sessionItem: WS* (docs=DOCS)? WS* (fwd=fwdCall | bwd=callBack | ctr=function) WS* ;
session: name=IDENTIFIER WS* '<' WS* items+=sessionItem (DECLSEP+ (items+=sessionItem)? WS*)*? '>';

item: WS* (docs=DOCS)? WS* (func=function | alias=typeAlias | sess=session) WS*;

rpc: items+=item (DECLSEP+ (items+=item | EOF))*;
 
PRIMITIVE:      [IiUu][1248];
IDENTIFIER:     [_a-zA-Z][_a-zA-Z0-9]*;
DOCS:			'/*' .*? '*/';
LISTSEP: 		WS* ',' WS*;
VALSEP:      	WS* ':' WS*;
DECLSEP:        WS* ';' WS*;
NAMEVALSEP:     WS* '=' WS*;
WS:             ('\r'? '\n') | [\t ];