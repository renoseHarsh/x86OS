#pragma once

#define ATOMIC_START() __asm__ volatile("pushfl; cli" ::: "memory")
#define ATOMIC_END() __asm__ volatile("popfl" ::: "memory")
