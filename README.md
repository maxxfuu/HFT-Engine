# Order Book and Trading Engine Plan 

Executive Summary: This plan outlines a project to build a high-performance C++17 trading engine in stages: first the Order Book, then Feed Handler, Matching Engine, and Market-Maker Strategy. 

When designing this project, the decisions made were considered through the lense of writing safe and performant C++. This includes using move semantics to avoid copies, thinking about RAII, and move sematics, referencing relevant concepts such as  cache-line alignment, using concurrency patterns, etc.