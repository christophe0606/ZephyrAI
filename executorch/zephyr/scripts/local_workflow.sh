#!/bin/bash

# Simple build script that executes the same commands as GitHub Actions workflow
# Updated for Zephyr module structure where executorch folder is mounted as /workspace2/executorch

set -e

WORKSPACE_ROOT="/workspace2"


# Setup logging
TIMESTAMP=$(date '+%Y%m%d_%H%M%S')
LOG_DIR="${WORKSPACE_ROOT}/logs"
MAIN_LOG="$LOG_DIR/build_${TIMESTAMP}.log"
mkdir -p "$LOG_DIR"

# Function to log with timestamp and tee to both console and log file
log_and_tee() {
    local step_name="$1"
    local log_file="$LOG_DIR/${step_name}_${TIMESTAMP}.log"
    echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] Starting: $step_name" | tee -a "$MAIN_LOG"
    
    # Execute the command and capture both stdout and stderr
    # Use PIPESTATUS to capture the exit code of the command before the pipe
    eval "$2" 2>&1 | tee -a "$log_file" "$MAIN_LOG"
    local exit_code=${PIPESTATUS[0]}
    
    if [ $exit_code -eq 0 ]; then
        echo "$(date '+%Y-%m-%d %H:%M:%S') [SUCCESS] Completed: $step_name" | tee -a "$MAIN_LOG"
        return 0
    else
        echo "$(date '+%Y-%m-%d %H:%M:%S') [ERROR] Failed: $step_name (exit code: $exit_code)" | tee -a "$MAIN_LOG"
        return 1
    fi
}

# Log build start
echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] ==================================" | tee "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] ExecuTorch AI Layer Build Started" | tee -a "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] Timestamp: $TIMESTAMP" | tee -a "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] Workspace Root: $WORKSPACE_ROOT" | tee -a "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] Log Directory: $LOG_DIR" | tee -a "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] ==================================" | tee -a "$MAIN_LOG"

# Step 2: Build ExecuTorch Core Libraries  
echo -e "\033[1;33m=== Step 2: Build ExecuTorch Core Libraries ===\033[0m"
log_and_tee "stage1_build" "${WORKSPACE_ROOT}/zephyr/scripts/build_stage1.sh /workspace/executorch ${WORKSPACE_ROOT}/out/stage1 ${WORKSPACE_ROOT}/model/arm-none-eabi-gcc.cmake"

# Step 2a: Generate source layer from stage1 compile_commands.json
# Note: posix.cpp is excluded because it uses std::chrono::steady_clock which is not available in ARM Clang bare-metal libc++
# Two strip paths are needed: one for ExecuTorch source, one for generated files in build dir
# Registration files (RegisterCodegenUnboxedKernelsEverything.cpp) are NOT included here - they come from stage2 (selective build)
# Flags -fPIC and -s are removed for AC6 compatibility (causes compiler crash in AC6 6.24.0)
echo -e "\033[1;35m=== Step 2a: Generate source layer ===\033[0m"
log_and_tee "generate_source_layer" "cd ${WORKSPACE_ROOT}/out/stage1 && python3 ${WORKSPACE_ROOT}/zephyr/scripts/ccdb2clayer.py -c compile_commands.json -v --copy-sources -A /workspace/executorch -A ${WORKSPACE_ROOT}/out/stage1 -L . -o ${WORKSPACE_ROOT}/stage1_source.clayer.yml -n 'ExecuTorch Stage1 Sources' -g runtime -g extension -g schema -g 'kernels/quantized' -g backends --exclude '*/posix.cpp' --remove-flags=-fPIC --remove-flags=-s"



# Step 3: Build ExecuTorch Selective Kernel Libraries
# Using model PTE for automatic operator analysis (instead of manual operators list)
echo -e "\033[1;34m=== Step 3: Build ExecuTorch Selective Kernel Libraries ===\033[0m"
log_and_tee "stage2_build" "${WORKSPACE_ROOT}/zephyr/scripts/build_stage2_selective.sh /workspace/executorch ${WORKSPACE_ROOT}/model/ethos_u_minimal_example.pte ${WORKSPACE_ROOT}/out/stage2 ${WORKSPACE_ROOT}/model/arm-none-eabi-gcc.cmake"

# Step 3a: Generate source layer from stage2 compile_commands.json
# Note: Files using std::random_device are excluded (not available in ARM Clang bare-metal libc++)
# Note: Quantized kernels are excluded because they're already in stage1_source.clayer.yml
# Two strip paths are needed: one for ExecuTorch source, one for generated files in build dir
# Flags -fPIC and -s are removed for AC6 compatibility (causes compiler crash in AC6 6.24.0)
echo -e "\033[1;35m=== Step 3a: Generate stage2 source layer ===\033[0m"
log_and_tee "generate_source_layer_stage2" "cd ${WORKSPACE_ROOT}/out/stage2 && python3 ${WORKSPACE_ROOT}/zephyr/scripts/ccdb2clayer.py -c compile_commands.json -v --copy-sources -A /workspace/executorch -A ${WORKSPACE_ROOT}/out/stage2 -L . -o ${WORKSPACE_ROOT}/stage2_source.clayer.yml -n 'ExecuTorch Stage2 Selective Ops' -g kernels --exclude '*/op_rand.cpp' --exclude '*/op_randn.cpp' --exclude '*/op_native_dropout.cpp' --exclude 'kernels/quantized/*' --remove-flags=-fPIC --remove-flags=-s"


# Step 4: Collect artifacts
echo -e "\033[1;32m=== Step 4: Collect artifacts ===\033[0m"
# Pass the ExecuTorch source path so package_sdk can copy extended_header.h.
log_and_tee "package_artifacts" "cd ${WORKSPACE_ROOT} && EXECUTORCH_SRC=/workspace/executorch sudo ./zephyr/scripts/package_sdk.sh \"${WORKSPACE_ROOT}/out/stage1/assets\" \"${WORKSPACE_ROOT}/out/stage2/assets\" \"${WORKSPACE_ROOT}/model/ethos_u_minimal_example.pte\" \"${WORKSPACE_ROOT}\""


# Step 5: Convert model to header file
echo -e "\033[1;31m=== Step 5: Convert model to header file ===\033[0m"
log_and_tee "model_to_header" "cd ${WORKSPACE_ROOT} && python3 zephyr/scripts/pte_to_header.py -p model/ethos_u_minimal_example.pte -d model -o model_pte.h"

# Step 6: Generate comprehensive AI layer report
echo -e "\033[1;93m=== Step 6: Generate AI layer report ===\033[0m"
log_and_tee "generate_report" "cd ${WORKSPACE_ROOT} && python3 zephyr/scripts/generate_ai_layer_report.py"

# Step 7: Build Report Summary
echo -e "\033[1;92m=== Step 7: Build Report Summary ===\033[0m" | tee -a "$MAIN_LOG"
echo "" | tee -a "$MAIN_LOG"
echo "📊 Build Summary:" | tee -a "$MAIN_LOG"
head -n 20 ${WORKSPACE_ROOT}/REPORT.md 2>/dev/null | tee -a "$MAIN_LOG" || echo "Report could not be displayed" | tee -a "$MAIN_LOG"

# Log build completion
echo "" | tee -a "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [SUCCESS] ==================================" | tee -a "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [SUCCESS] ExecuTorch AI Layer Build Completed" | tee -a "$MAIN_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') [SUCCESS] ==================================" | tee -a "$MAIN_LOG"
echo "" | tee -a "$MAIN_LOG"
echo "📋 Build logs available at: $LOG_DIR" | tee -a "$MAIN_LOG"
echo "📋 Main build log: $MAIN_LOG" | tee -a "$MAIN_LOG"
echo "📋 Full report available at: REPORT.md" | tee -a "$MAIN_LOG"

# Create a build summary file
cat > "$LOG_DIR/build_summary_${TIMESTAMP}.txt" << EOF
ExecuTorch AI Layer Build Summary
=================================
Build Timestamp: $TIMESTAMP
Workspace Root: $WORKSPACE_ROOT
Build Status: SUCCESS
Build Duration: Started at $(head -n 1 "$MAIN_LOG" | cut -d' ' -f1-2)

Log Files Generated:
- Main build log: $MAIN_LOG
- Model conversion: $LOG_DIR/model_conversion_${TIMESTAMP}.log
- Stage1 build: $LOG_DIR/stage1_build_${TIMESTAMP}.log
- Generate source layer: $LOG_DIR/generate_source_layer_${TIMESTAMP}.log
- Stage2 build: $LOG_DIR/stage2_build_${TIMESTAMP}.log
- Package artifacts: $LOG_DIR/package_artifacts_${TIMESTAMP}.log
- Model to header: $LOG_DIR/model_to_header_${TIMESTAMP}.log
- Generate report: $LOG_DIR/generate_report_${TIMESTAMP}.log

Outputs Generated:
- AI Layer libraries: executorch/lib/
- AI Layer headers: executorch/include/
- Model header: executorch/model_pte.h
- Build report: executorch/REPORT.md

For detailed information, see the individual log files above.
EOF

echo ""
echo "✅ Build completed successfully!"
echo "📊 Summary file: $LOG_DIR/build_summary_${TIMESTAMP}.txt"
