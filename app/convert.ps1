Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile('D:\ProgramData\GitHub\acaddia-web2\app\schematic.png')
Write-Output ("orig: " + $img.Width + "x" + $img.Height)
$bmp = New-Object System.Drawing.Bitmap $img
$bmp.Save('D:\ProgramData\GitHub\acaddia-web2\app\schematic.jpg', [System.Drawing.Imaging.ImageFormat]::Jpeg)
$bmp.Dispose(); $img.Dispose()
Write-Output 'converted'
