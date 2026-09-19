[CmdletBinding()]
param(
    [string]$RepositoryPath,

    [ValidateNotNullOrEmpty()]
    [string]$RemoteUrl = 'git@github.com:liqi1222/U585.git'
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($RepositoryPath)) {
    $RepositoryPath = Split-Path -Parent $PSScriptRoot
}

function ConvertTo-ProcessArgument {
    param(
        [Parameter(Mandatory)]
        [string]$Value
    )

    '"' + $Value.Replace('"', '\"') + '"'
}

function Invoke-Git {
    param(
        [Parameter(Mandatory)]
        [string]$Repository,

        [Parameter(Mandatory)]
        [string[]]$Arguments,

        [switch]$AllowFailure
    )

    $processStartInfo = New-Object System.Diagnostics.ProcessStartInfo
    $processStartInfo.FileName = 'git'
    $processStartInfo.WorkingDirectory = $Repository
    $processStartInfo.Arguments = (($Arguments | ForEach-Object { ConvertTo-ProcessArgument $_ }) -join ' ')
    $processStartInfo.UseShellExecute = $false
    $processStartInfo.CreateNoWindow = $true
    $processStartInfo.RedirectStandardOutput = $true
    $processStartInfo.RedirectStandardError = $true

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $processStartInfo
    [void]$process.Start()
    $standardOutput = $process.StandardOutput.ReadToEnd()
    $standardError = $process.StandardError.ReadToEnd()
    $process.WaitForExit()

    $result = [PSCustomObject]@{
        ExitCode = $process.ExitCode
        Output = "$standardOutput$standardError"
    }

    if (-not $AllowFailure -and $result.ExitCode -ne 0) {
        throw "git $($Arguments -join ' ') failed in '$Repository': $($result.Output)"
    }

    return $result
}

if (-not (Test-Path -LiteralPath $RepositoryPath -PathType Container)) {
    throw "Repository path does not exist or is not a directory: $RepositoryPath"
}

$repositoryPath = (Resolve-Path -LiteralPath $RepositoryPath).Path
$worktreeResult = Invoke-Git -Repository $repositoryPath -Arguments @('rev-parse', '--is-inside-work-tree') -AllowFailure
if ($worktreeResult.ExitCode -ne 0 -or $worktreeResult.Output.Trim() -ne 'true') {
    throw "Repository path is not a Git worktree: $repositoryPath"
}

$mainResult = Invoke-Git -Repository $repositoryPath -Arguments @('show-ref', '--verify', '--quiet', 'refs/heads/main') -AllowFailure
if ($mainResult.ExitCode -ne 0) {
    # An initialized repository can have main checked out before its first commit.
    $headReference = Invoke-Git -Repository $repositoryPath -Arguments @('symbolic-ref', '--quiet', 'HEAD') -AllowFailure
    if ($headReference.ExitCode -ne 0 -or $headReference.Output.Trim() -ne 'refs/heads/main') {
        throw "The Git worktree does not have a local main branch: $repositoryPath"
    }
}

$hookPath = Join-Path $repositoryPath '.githooks/post-commit'
if (-not (Test-Path -LiteralPath $hookPath -PathType Leaf)) {
    throw "Required publish hook does not exist: $hookPath"
}

$remoteNames = (Invoke-Git -Repository $repositoryPath -Arguments @('remote')).Output -split "`r?`n" | Where-Object { $_ }
if ($remoteNames -contains 'github') {
    Invoke-Git -Repository $repositoryPath -Arguments @('remote', 'set-url', 'github', $RemoteUrl) | Out-Null
}
else {
    Invoke-Git -Repository $repositoryPath -Arguments @('remote', 'add', 'github', $RemoteUrl) | Out-Null
}

$remoteHeads = (Invoke-Git -Repository $repositoryPath -Arguments @('ls-remote', '--heads', 'github', 'refs/heads/develop')).Output
$developExists = $false
foreach ($line in $remoteHeads -split "`r?`n") {
    if ($line -match '^\S+\s+refs/heads/develop$') {
        $developExists = $true
        break
    }
}

if ($developExists) {
    Invoke-Git -Repository $repositoryPath -Arguments @('fetch', 'github', 'refs/heads/develop:refs/remotes/github/develop') | Out-Null
    $ancestryResult = Invoke-Git -Repository $repositoryPath -Arguments @('merge-base', '--is-ancestor', 'refs/remotes/github/develop', 'main') -AllowFailure

    if ($ancestryResult.ExitCode -eq 1) {
        throw 'GitHub develop diverges from local main. Resolve the branch histories before enabling automatic publishing; core.hooksPath was not changed.'
    }
    if ($ancestryResult.ExitCode -ne 0) {
        throw "Unable to compare GitHub develop with local main: $($ancestryResult.Output)"
    }
}

Invoke-Git -Repository $repositoryPath -Arguments @('config', 'core.hooksPath', '.githooks') | Out-Null
Write-Host "GitHub publish hook is enabled for '$repositoryPath'. Local main commits will publish to github/develop."
