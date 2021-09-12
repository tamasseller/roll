grammar rpc;

primitive:  kind=PRIMITIVE;
collection: '[' WS* elementType=typeref WS* ']';
typeref: p=primitive | c=collection | n=IDENTIFIER;
var: WS* (docs=DOCS)? WS* name=IDENTIFIER VALSEP t=typeref  WS*;
varList: WS* vars+=var? (LISTSEP vars+=var)* WS* ;
action: name=IDENTIFIER WS* '(' args=varList ')';
function: call=action (VALSEP ret=typeref)?;

aggregate:  '{' members=varList '}';
typeAlias: name=IDENTIFIER NAMEVALSEP (p=primitive | a=aggregate | c=collection | n=IDENTIFIER);

fwdCall: '!' WS* sym=action;
callBack: '@' WS* sym=action;
sessionItem: WS* (docs=DOCS)? WS* (fwd=fwdCall | bwd=callBack | ctr=function) WS* ;
session: name=IDENTIFIER WS* '<' WS* items+=sessionItem (DECLSEP+ (items+=sessionItem)? WS*)*? '>';

contract: '$' WS* name=IDENTIFIER;

item: WS* (docs=DOCS)? WS* (cont=contract | func=function | alias=typeAlias | sess=session) WS*;
rpc: items+=item (DECLSEP+ (items+=item | EOF))*;

PRIMITIVE:      ([IiUu][1248]|'bool');
IDENTIFIER:     [_a-zA-Z][_a-zA-Z0-9]*;
DOCS:			'/*' .*? '*/';
LISTSEP: 		WS* ',' WS*;
VALSEP:      	WS* ':' WS*;
DECLSEP:        WS* ';' WS*;
NAMEVALSEP:     WS* '=' WS*;
WS:             ('\r'? '\n') | [\t ];
COMMENT: 		'#' ~[\r\n]* '\n';