#ifndef _UC32SIM_PIPELINE_H
#define _UC32SIM_PIPELINE_H
/* 
 * Pipeline Design:
 * 		5 pipeline stages (like MIPS)
 * 1. Instruction Fetch
 * 2. Instruction Decode
 * 3. Execution
 * 4. Memory Access
 * 5. Write-back
 * 
 * Hazard detection
 * Pipeline Hazards
 * 1. Structual Hazards
 * 		Seperate Instruction/Data cache will eliminate structual hazards 
 *		on memory access.
 * 2. Data Hazards
 *		Forwarding
 *		Pipeline stall / Bubble
 * 3. Control Hazards
 *		Static/ dynamic branch prediction
 *		Flush the pipeline on misprediction
 *
 */
#endif /* !_UC32SIM_PIPELINE_H */