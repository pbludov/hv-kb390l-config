$packageName = 'hv-kb390l-config'
$installerType = 'msi'
$url = 'https://github.com/pbludov/hv-kb390l-config/releases/download/v1.1.0/hv-kb390l-config.msi'
$silentArgs = '/Q'
$validExitCodes = @(0)
$checksum = 'a6f463617ad62eb15f5138d34dc877297c2c05771ca5738e51f4f4117fe530af'
$checksumType = 'sha256'

Install-ChocolateyPackage "$packageName" "$installerType" "$silentArgs" "$url"  -validExitCodes $validExitCodes -Checksum $checksum -ChecksumType $checksumType
