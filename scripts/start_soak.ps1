param(
    [Parameter(Mandatory=$true)][string]$Url,
    [Parameter(Mandatory=$true)][string]$Mac,
    [Parameter(Mandatory=$true)][string]$Version,
    [Parameter(Mandatory=$true)][string]$BuildUtc,
    [Parameter(Mandatory=$true)][string]$Output,
    [double]$Hours=24,
    [double]$ScenarioSeconds=300,
    [double[]]$AcceptedPose=@()
)
$ErrorActionPreference='Stop'
$repoRoot=Split-Path -Parent $PSScriptRoot
$outputPath=[System.IO.Path]::GetFullPath((Join-Path $repoRoot $Output))
if(Test-Path -LiteralPath $outputPath){throw 'Output already exists; never start a duplicate/replacement job'}
$logParent=Split-Path -Parent $outputPath
New-Item -ItemType Directory -Path $logParent -Force | Out-Null
$pythonPath=(Get-Command python -ErrorAction Stop).Source
$scriptPath=Join-Path $PSScriptRoot 'soak.py'
$runnerArgs=@('-u',('"'+$scriptPath+'"'),'--url',$Url,'--mac',$Mac,'--version',$Version,
              '--build',$BuildUtc,'--hours',$Hours.ToString([cultureinfo]::InvariantCulture),
              '--scenario-seconds',$ScenarioSeconds.ToString([cultureinfo]::InvariantCulture),
              '--output',('"'+$outputPath+'"'))
if($AcceptedPose.Count){
    if($AcceptedPose.Count -ne 2){throw 'AcceptedPose must contain exactly two angles'}
    $runnerArgs+=@('--accepted-pose')
    foreach($poseAngle in $AcceptedPose){$runnerArgs+=$poseAngle.ToString([cultureinfo]::InvariantCulture)}
}
$runnerProcess=Start-Process -FilePath $pythonPath -ArgumentList $runnerArgs -WorkingDirectory $repoRoot `
    -WindowStyle Hidden -RedirectStandardOutput ($outputPath+'.stdout.log') `
    -RedirectStandardError ($outputPath+'.stderr.log') -PassThru
$launchInfo=[ordered]@{pid=$runnerProcess.Id;started_utc=[datetime]::UtcNow.ToString('o');output=$outputPath}
$launchInfo | ConvertTo-Json | Set-Content -LiteralPath ($outputPath+'.launch.json')
$launchInfo | ConvertTo-Json
