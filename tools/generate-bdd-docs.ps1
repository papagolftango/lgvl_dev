param(
    [string]$FeaturesRoot = "tests/bdd/features",
    [string]$DocsOut = "docs/bdd"
)

Write-Host "Generating BDD docs from $FeaturesRoot to $DocsOut" -ForegroundColor Cyan

# Ensure output directory exists
New-Item -ItemType Directory -Force -Path $DocsOut | Out-Null

# Build an index content
$indexLines = @("# BDD Feature Index","","These pages are generated from .feature files under `$FeaturesRoot`.","","## Features")

# Gather feature files
$featureFiles = Get-ChildItem -Path $FeaturesRoot -Recurse -Filter *.feature | Sort-Object FullName

if (!$featureFiles) {
    Write-Warning "No .feature files found under $FeaturesRoot"
}

foreach ($file in $featureFiles) {
    $cwd = (Resolve-Path ".").Path
    try {
        # PowerShell 7+ with .NET Core
        $relPath = [System.IO.Path]::GetRelativePath($cwd, $file.FullName)
    } catch {
        # PowerShell 5.1 fallback
        $relPath = (Resolve-Path -Relative $file.FullName)
        # Remove leading .\ or ./
        $relPath = $relPath -replace '^[.][\\/]', ''
    }
    # Normalize to forward slashes
    $relPath = $relPath -replace "\\", "/"
    $relPretty = $relPath -replace "^tests/bdd/features/", ""

    # Read content
    $content = Get-Content -Raw -Path $file.FullName

    # Convert to Markdown using here-string; use tildes to avoid PowerShell backtick escapes
    $baseName = [IO.Path]::GetFileNameWithoutExtension($file.Name)
    $md = @"
# $baseName

Source: $relPath

~~~gherkin
$content
~~~
"@

    # Determine output path under docs/bdd mirroring feature folder structure
    $relativeFolder = Split-Path -Path $relPretty -Parent
    $outFolder = Join-Path $DocsOut $relativeFolder
    New-Item -ItemType Directory -Force -Path $outFolder | Out-Null

    $outFile = Join-Path $outFolder ($baseName + ".md")
    Set-Content -Path $outFile -Value $md -Encoding UTF8

    # Add to index
    $linkPath = $relPretty.replace('.feature','.md')
    $indexLines += "- [$relPretty]($linkPath)"
}

# Write index.md
$indexPath = Join-Path $DocsOut "index.md"
$indexBody = $indexLines -join "`n"
Set-Content -Path $indexPath -Value $indexBody -Encoding UTF8

Write-Host "Done. Wrote $($featureFiles.Count) feature(s)." -ForegroundColor Green
