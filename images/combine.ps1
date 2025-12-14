Add-Type -AssemblyName System.Drawing

$L = [System.Drawing.Bitmap]::new("C:\Users\rpadi\Documents\Dev\Penumbra\images\1L.png")
$R = [System.Drawing.Bitmap]::new("C:\Users\rpadi\Documents\Dev\Penumbra\images\1R.png")
$O = [System.Drawing.Bitmap]::new($L.Width, $L.Height)

for ($y = 0; $y -lt $L.Height; $y++) {
    for ($x = 0; $x -lt $L.Width; $x++) {
        $cL = $L.GetPixel($x, $y)
        $cR = $R.GetPixel($x, $y)
        $O.SetPixel($x, $y,
            [System.Drawing.Color]::FromArgb($cL.R, $cR.G, $cR.B))
    }
}

$O.Save("C:\Users\rpadi\Documents\Dev\Penumbra\images\1.png")
