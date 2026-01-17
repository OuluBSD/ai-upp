<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>@TARGET_FRAMEWORK@</TargetFramework>
    <TargetPlatformMinVersion>@TARGET_PLATFORM_MIN_VERSION@</TargetPlatformMinVersion>
    <RootNamespace>@ROOT_NAMESPACE@</RootNamespace>
    <AssemblyName>@PROJECT_NAME@</AssemblyName>
    <GenerateAssemblyInfo>false</GenerateAssemblyInfo>
  </PropertyGroup>
  <ItemGroup>
    <AppxManifest Include="Package.appxmanifest" />
  </ItemGroup>
  <ItemGroup>
@CSCOMPILE@
  </ItemGroup>
  <ItemGroup>
@NONEITEMS@
  </ItemGroup>
  <ItemGroup>
@CONTENT_ITEMS@
  </ItemGroup>
</Project>
