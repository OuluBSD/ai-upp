<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>@TARGET_FRAMEWORK@</TargetFramework>
    <RootNamespace>@ROOT_NAMESPACE@</RootNamespace>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
  </PropertyGroup>
  <ItemGroup>
@CSCOMPILE@
  </ItemGroup>
  <ItemGroup>
@NONEITEMS@
  </ItemGroup>
</Project>
