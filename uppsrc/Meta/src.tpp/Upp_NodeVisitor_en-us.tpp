topic "";
[H6;0 $$1,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$2,0#37138531426314131252341829483370:codeitem]
[l288;2 $$3,0#27521748481378242620020725143825:desc]
[0 $$4,0#96390100711032703541132217272105:end]
[ $$0,0#00000000000000000000000000000000:Default]
[{_} 
[s1; &]
[s2;:Upp`:`:NodeVisitor`:`:Ver`(int`): NodeVisitor[@(0.0.255) `&] [* Ver]([@(0.0.255) int] 
[*@3 version])&]
[s3;%%  [%-*@3 version] .current version of the class&]
[s4; &]
[s1;%% This is a function that determines the current version of 
the class being visited at the start of the Visit function. It 
is compared to the version number of the values ​​read, and 
newer values ​​are not read. Only the same version or previous 
versions are read.&]
[s2;:Upp`:`:NodeVisitor`:`:operator`(`)`(int`): NodeVisitor[@(0.0.255) `&] 
operator()([@(0.0.255) int] version)&]
[s3;%% [%-*@3 version]  .version number for the following variables 
to be visited&]
[s4; &]
[s0;%% This function determines the version number of the following 
variables. It is compared to the class version and if the version 
being read is older, the variables are not visited.]]