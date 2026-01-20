<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{@PROJECT_GUID@}</ProjectGuid>
    <Keyword>WindowsApp</Keyword>
    <RootNamespace>@ROOT_NAMESPACE@</RootNamespace>
    <DefaultLanguage>en-US</DefaultLanguage>
    <AppContainerApplication>true</AppContainerApplication>
    <ApplicationType>Windows Store</ApplicationType>
    <ApplicationTypeRevision>10.0</ApplicationTypeRevision>
    <WindowsTargetPlatformVersion>@TARGET_PLATFORM_VERSION@</WindowsTargetPlatformVersion>
    <WindowsTargetPlatformMinVersion>@TARGET_PLATFORM_MIN_VERSION@</WindowsTargetPlatformMinVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>@CONFIGURATION_TYPE@</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>@PLATFORM_TOOLSET@</PlatformToolset>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>@CONFIGURATION_TYPE@</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>@PLATFORM_TOOLSET@</PlatformToolset>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <OutDir>$(SolutionDir)AppX\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)AppX\Intermediate\$(Configuration)\@PROJECT_NAME@\</IntDir>
@APPX_PACKAGE@
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <OutDir>$(SolutionDir)AppX\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)AppX\Intermediate\$(Configuration)\@PROJECT_NAME@\</IntDir>
@APPX_PACKAGE@
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <AdditionalIncludeDirectories>@INCLUDE_DIRS@</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>@DEFINES@</PreprocessorDefinitions>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <LanguageStandard>stdcpp17</LanguageStandard>
      <CompileAsWinRT>false</CompileAsWinRT>
    </ClCompile>
@LINK_SETTINGS@
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <AdditionalIncludeDirectories>@INCLUDE_DIRS@</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>@DEFINES@</PreprocessorDefinitions>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <LanguageStandard>stdcpp17</LanguageStandard>
      <CompileAsWinRT>false</CompileAsWinRT>
    </ClCompile>
@LINK_SETTINGS@
  </ItemDefinitionGroup>
  <ItemGroup>
@CLCOMPILE@
  </ItemGroup>
  <ItemGroup>
@CLINCLUDE@
  </ItemGroup>
  <ItemGroup>
@NONEITEMS@
  </ItemGroup>
  <ItemGroup>
@CONTENT_ITEMS@
  </ItemGroup>
@PROJECT_REFERENCES@
@APPX_MANIFEST_ITEM@
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
</Project>
