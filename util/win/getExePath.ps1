Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.IO

$OpenFileDialog = New-Object System.Windows.Forms.OpenFileDialog
$OpenFileDialog.InitialDirectory = Join-Path -Path [System.IO.Path]::GetDirectoryName($MyInvocation.MyCommand.Definition) -ChildPath "..\..\..\build\win11\qtcreator\release\"
$OpenFileDialog.FileName = "helios-launcher.exe"
$OpenFileDialog.Filter = "Applications (*.exe)|*.exe"
$OpenFileDialog.ShowDialog() | Out-Null

Write-Output $OpenFileDialog.FileName
