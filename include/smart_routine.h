#pragma once

int smart_routine_start(void *(*fn)(void *), void *args);

int smart_routine_yeild();