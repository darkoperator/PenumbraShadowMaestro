# Generate a random 16-byte ESPNOW LMK as 32 hex characters
param(
  [int]$Count = 1
)

function New-LmkHex {
  $rng = [System.Security.Cryptography.RandomNumberGenerator]::Create()
  $bytes = New-Object byte[] 16
  $rng.GetBytes($bytes)
  ($bytes | ForEach-Object { $_.ToString('X2') }) -join ''
}

1..$Count | ForEach-Object { New-LmkHex }

