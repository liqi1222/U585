[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

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

        [Parameter(ValueFromRemainingArguments)]
        [string[]]$Arguments
    )

    $processStartInfo = New-Object System.Diagnostics.ProcessStartInfo
    $processStartInfo.FileName = 'git'
    $processStartInfo.WorkingDirectory = $Repository
    $processStartInfo.Arguments = (($Arguments | ForEach-Object { '"' + $_.Replace('"', '\"') + '"' }) -join ' ')
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
    $exitCode = $process.ExitCode

    if ($exitCode -ne 0) {
        throw "git $($Arguments -join ' ') failed in '$Repository': $standardOutput$standardError"
    }

    $output = "$standardOutput$standardError"
    if (-not [string]::IsNullOrEmpty($output)) {
        return $output
    }
}

function Invoke-PowerShellFile {
    param(
        [Parameter(Mandatory)]
        [string]$ScriptPath,

        [Parameter(ValueFromRemainingArguments)]
        [string[]]$Arguments
    )

    $processStartInfo = New-Object System.Diagnostics.ProcessStartInfo
    $processStartInfo.FileName = (Get-Command powershell.exe -ErrorAction Stop).Source
    $processStartInfo.WorkingDirectory = Split-Path -Parent $ScriptPath
    $processStartInfo.Arguments = ((@('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $ScriptPath) + $Arguments | ForEach-Object { ConvertTo-ProcessArgument $_ }) -join ' ')
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

    if ($process.ExitCode -ne 0) {
        throw "powershell -File $ScriptPath $($Arguments -join ' ') failed: $standardOutput$standardError"
    }

    $output = "$standardOutput$standardError"
    if (-not [string]::IsNullOrEmpty($output)) {
        return $output
    }
}

function Assert-Equal {
    param(
        [Parameter(Mandatory)]
        [string]$Expected,

        [Parameter(Mandatory)]
        [string]$Actual,

        [Parameter(Mandatory)]
        [string]$Message
    )

    if ($Expected -ne $Actual) {
        throw "$Message Expected '$Expected', got '$Actual'."
    }
}

$repositoryRoot = Split-Path -Parent $PSScriptRoot
$sourceHook = Join-Path $repositoryRoot '.githooks/post-commit'
$sourceInstaller = Join-Path $repositoryRoot 'tools/setup_github_publish.ps1'
$temporaryRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("u585-github-publish-hook-" + [System.Guid]::NewGuid())
$githubBareRepository = Join-Path $temporaryRoot 'github.git'
$placeholderGithubBareRepository = Join-Path $temporaryRoot 'placeholder-github.git'
$workRepository = Join-Path $temporaryRoot 'work'
$defaultInvocationRepository = Join-Path $temporaryRoot 'default-invocation-repository'
$defaultInvocationToolsDirectory = Join-Path $defaultInvocationRepository 'tools'
$defaultInvocationHooksDirectory = Join-Path $defaultInvocationRepository '.githooks'

New-Item -ItemType Directory -Path $temporaryRoot | Out-Null

try {
    if (-not (Test-Path -LiteralPath $sourceHook -PathType Leaf)) {
        throw "Required source hook does not exist: $sourceHook"
    }
    if (-not (Test-Path -LiteralPath $sourceInstaller -PathType Leaf)) {
        throw "Required source installer does not exist: $sourceInstaller"
    }

    Invoke-Git -Repository $temporaryRoot init --bare $githubBareRepository | Out-Null
    Invoke-Git -Repository $temporaryRoot init --bare $placeholderGithubBareRepository | Out-Null

    New-Item -ItemType Directory -Path $defaultInvocationToolsDirectory, $defaultInvocationHooksDirectory | Out-Null
    Copy-Item -LiteralPath $sourceInstaller -Destination (Join-Path $defaultInvocationToolsDirectory 'setup_github_publish.ps1')
    Copy-Item -LiteralPath $sourceHook -Destination (Join-Path $defaultInvocationHooksDirectory 'post-commit')
    Invoke-Git -Repository $temporaryRoot init --initial-branch=main $defaultInvocationRepository | Out-Null

    $copiedInstaller = Join-Path $defaultInvocationToolsDirectory 'setup_github_publish.ps1'
    Invoke-PowerShellFile -ScriptPath $copiedInstaller -RemoteUrl $githubBareRepository | Out-Null

    $defaultConfiguredGithubRemoteUrl = (Invoke-Git -Repository $defaultInvocationRepository remote get-url github).Trim()
    Assert-Equal -Expected $githubBareRepository -Actual $defaultConfiguredGithubRemoteUrl -Message 'The installer did not configure the repository inferred from its default path.'
    $defaultConfiguredHooksPath = (Invoke-Git -Repository $defaultInvocationRepository config --get core.hooksPath).Trim()
    Assert-Equal -Expected '.githooks' -Actual $defaultConfiguredHooksPath -Message 'The default installer invocation did not configure the Git hooks path.'

    Invoke-Git -Repository $temporaryRoot init --initial-branch=main $workRepository | Out-Null
    Invoke-Git -Repository $workRepository config user.name 'U585 hook test'
    Invoke-Git -Repository $workRepository config user.email 'u585-hook-test@example.invalid'

    $workHooksDirectory = Join-Path $workRepository '.githooks'
    New-Item -ItemType Directory -Path $workHooksDirectory | Out-Null
    Copy-Item -LiteralPath $sourceHook -Destination (Join-Path $workHooksDirectory 'post-commit')
    & $sourceInstaller -RepositoryPath $workRepository -RemoteUrl $githubBareRepository

    $configuredGithubRemoteUrl = (Invoke-Git -Repository $workRepository remote get-url github).Trim()
    Assert-Equal -Expected $githubBareRepository -Actual $configuredGithubRemoteUrl -Message 'The GitHub remote URL was not configured by the installer.'
    $configuredHooksPath = (Invoke-Git -Repository $workRepository config --get core.hooksPath).Trim()
    Assert-Equal -Expected '.githooks' -Actual $configuredHooksPath -Message 'The Git hooks path was not configured by the installer.'

    Invoke-Git -Repository $workRepository remote set-url github $placeholderGithubBareRepository
    & $sourceInstaller -RepositoryPath $workRepository -RemoteUrl $githubBareRepository
    $restoredGithubRemoteUrl = (Invoke-Git -Repository $workRepository remote get-url github).Trim()
    Assert-Equal -Expected $githubBareRepository -Actual $restoredGithubRemoteUrl -Message 'The installer did not restore the existing GitHub remote URL.'

    Set-Content -LiteralPath (Join-Path $workRepository 'README.md') -Value 'main commit'
    Invoke-Git -Repository $workRepository add README.md
    Invoke-Git -Repository $workRepository commit -m 'test: publish main to develop' | Out-Null

    $localMainHead = (Invoke-Git -Repository $workRepository rev-parse HEAD).Trim()
    $remoteDevelopHead = (Invoke-Git -Repository $githubBareRepository rev-parse refs/heads/develop).Trim()
    Assert-Equal -Expected $localMainHead -Actual $remoteDevelopHead -Message 'The main commit was not published to github/develop.'

    Set-Content -LiteralPath (Join-Path $workRepository 'README.md') -Value 'second main commit'
    Invoke-Git -Repository $workRepository add README.md
    Invoke-Git -Repository $workRepository commit -m 'test: publish a second main commit to develop' | Out-Null

    $secondLocalMainHead = (Invoke-Git -Repository $workRepository rev-parse HEAD).Trim()
    $remoteDevelopAfterSecondMainCommit = (Invoke-Git -Repository $githubBareRepository rev-parse refs/heads/develop).Trim()
    Assert-Equal -Expected $secondLocalMainHead -Actual $remoteDevelopAfterSecondMainCommit -Message 'The second main commit was not published to github/develop.'
    if ($remoteDevelopHead -eq $remoteDevelopAfterSecondMainCommit) {
        throw 'The second main commit did not advance github/develop.'
    }

    $remoteWriterRepository = Join-Path $temporaryRoot 'remote-writer'
    Invoke-Git -Repository $temporaryRoot clone --branch develop $githubBareRepository $remoteWriterRepository | Out-Null
    Invoke-Git -Repository $remoteWriterRepository config user.name 'U585 remote writer'
    Invoke-Git -Repository $remoteWriterRepository config user.email 'u585-remote-writer@example.invalid'
    Set-Content -LiteralPath (Join-Path $remoteWriterRepository 'remote-only.txt') -Value 'divergent remote commit'
    Invoke-Git -Repository $remoteWriterRepository add remote-only.txt
    Invoke-Git -Repository $remoteWriterRepository commit -m 'test: diverge github develop' | Out-Null
    Invoke-Git -Repository $remoteWriterRepository push origin HEAD:develop | Out-Null

    $divergentRemoteDevelopHead = (Invoke-Git -Repository $githubBareRepository rev-parse refs/heads/develop).Trim()
    if ($divergentRemoteDevelopHead -eq $remoteDevelopAfterSecondMainCommit) {
        throw 'The temporary remote did not diverge github/develop.'
    }

    Invoke-Git -Repository $workRepository config core.hooksPath '.hooks-sentinel'
    $installerDivergenceError = $null
    try {
        & $sourceInstaller -RepositoryPath $workRepository -RemoteUrl $githubBareRepository
    }
    catch {
        $installerDivergenceError = $_.Exception.Message
    }
    if ($null -eq $installerDivergenceError -or $installerDivergenceError -notmatch 'GitHub develop diverges') {
        throw "The installer did not report the expected GitHub develop divergence: $installerDivergenceError"
    }
    $hooksPathAfterInstallerDivergence = (Invoke-Git -Repository $workRepository config --get core.hooksPath).Trim()
    Assert-Equal -Expected '.hooks-sentinel' -Actual $hooksPathAfterInstallerDivergence -Message 'The installer changed core.hooksPath after detecting divergence.'
    Invoke-Git -Repository $workRepository config core.hooksPath '.githooks'

    Set-Content -LiteralPath (Join-Path $workRepository 'README.md') -Value 'third main commit after remote divergence'
    Invoke-Git -Repository $workRepository add README.md
    $failedPublishCommitOutput = Invoke-Git -Repository $workRepository commit -m 'test: keep local commit when github develop diverges'

    $localMainHeadAfterFailedPublish = (Invoke-Git -Repository $workRepository rev-parse HEAD).Trim()
    $localMainCommitSubject = (Invoke-Git -Repository $workRepository log -1 --format=%s).Trim()
    Assert-Equal -Expected 'test: keep local commit when github develop diverges' -Actual $localMainCommitSubject -Message 'The local main commit did not complete after the rejected publish.'
    if ($localMainHeadAfterFailedPublish -eq $divergentRemoteDevelopHead) {
        throw 'The rejected publish unexpectedly made the local main commit equal github/develop.'
    }
    $remoteDevelopAfterRejectedPublish = (Invoke-Git -Repository $githubBareRepository rev-parse refs/heads/develop).Trim()
    Assert-Equal -Expected $divergentRemoteDevelopHead -Actual $remoteDevelopAfterRejectedPublish -Message 'A rejected publish changed github/develop.'
    if ($failedPublishCommitOutput -notmatch 'GitHub publish failed') {
        throw 'The rejected publish did not report the GitHub publish failure.'
    }

    Invoke-Git -Repository $workRepository checkout -b feature/test | Out-Null
    Set-Content -LiteralPath (Join-Path $workRepository 'feature.txt') -Value 'feature commit'
    Invoke-Git -Repository $workRepository add feature.txt
    Invoke-Git -Repository $workRepository commit -m 'test: do not publish feature branches' | Out-Null

    $remoteDevelopAfterFeatureCommit = (Invoke-Git -Repository $githubBareRepository rev-parse refs/heads/develop).Trim()
    Assert-Equal -Expected $divergentRemoteDevelopHead -Actual $remoteDevelopAfterFeatureCommit -Message 'A feature branch commit changed github/develop.'

    Write-Host 'GitHub publish hook integration test passed.'
}
finally {
    if (Test-Path -LiteralPath $temporaryRoot) {
        Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
    }
}
