param(
  [int]$Port = 8000
)

# Ensure docs are up to date
./tools/generate-bdd-docs.ps1

# Try to use mkdocs if available
try {
  $mk = Get-Command mkdocs -ErrorAction Stop
  Write-Host "Serving with mkdocs on http://127.0.0.1:$Port" -ForegroundColor Cyan
  mkdocs serve -a 127.0.0.1:$Port
} catch {
  Write-Host "mkdocs not found. Serving raw docs/ via simple HTTP server (Python if available)." -ForegroundColor Yellow
  try {
    $py = Get-Command python -ErrorAction Stop
    Set-Location docs
    python -m http.server $Port
  } catch {
    Write-Error "Neither mkdocs nor python are available. Please install mkdocs (pip install mkdocs) or Python."
  }
}
