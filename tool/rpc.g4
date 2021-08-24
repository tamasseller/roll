grammar rpc;

rpc: WS* items+=item (DECLSEP+ items+=item)* WS*;

item: push=symbol | pull=getter | EOF;
getter: sym=symbol RETVALSEP ret=type;
symbol: name=IDENTIFIER WS* '(' args=varList ')';

varList: vars+=var? (LIST_SEPARATOR vars+=var)* ;
var: t=type WS+ name=IDENTIFIER;
type: p=primitive | a=aggregate | c=collection;

primitive:  kind=PRIMITIVE;
collection: '[' elementType=type ']';
aggregate:  '{' members=varList '}';
 
PRIMITIVE:      [IiUu][1248];
IDENTIFIER:     [_a-zA-Z][_a-zA-Z0-9]*;
LIST_SEPARATOR: WS* ',' WS*;
RETVALSEP:      WS* ':' WS*;
DECLSEP:        WS* ';' WS*;
WS:             ('\r'? '\n') | [\t ];