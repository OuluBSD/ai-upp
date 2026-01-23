<?xml version="1.0" encoding="utf-8"?>
<Package
  xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
  xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"
  xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
  IgnorableNamespaces="uap mp">
  <Identity Name="@PACKAGE_NAME@" Publisher="CN=UppUwp" Version="@PACKAGE_VERSION@" />
  <mp:PhoneIdentity PhoneProductId="@PHONE_PRODUCT_ID@" PhonePublisherId="@PHONE_PUBLISHER_ID@" />
  <Properties>
    <DisplayName>@PROJECT_NAME@</DisplayName>
    <PublisherDisplayName>Upp</PublisherDisplayName>
    <Logo>Assets\StoreLogo.png</Logo>
  </Properties>
  <Dependencies>
    <TargetDeviceFamily Name="Windows.Universal" MinVersion="@TARGET_PLATFORM_MIN_VERSION@" MaxVersionTested="@TARGET_PLATFORM_VERSION@" />
  </Dependencies>
  <Resources>
    <Resource Language="en-us" />
  </Resources>
  <Applications>
    <Application Id="App" Executable="$targetnametoken$.exe" EntryPoint="@ENTRY_POINT@">
      <uap:VisualElements DisplayName="@PROJECT_NAME@" Square150x150Logo="Assets\Square150x150Logo.png" Square44x44Logo="Assets\Square44x44Logo.png" Description="@PROJECT_NAME@" BackgroundColor="transparent">
        <uap:DefaultTile Wide310x150Logo="Assets\Wide310x150Logo.png" />
      </uap:VisualElements>
    </Application>
  </Applications>
</Package>
