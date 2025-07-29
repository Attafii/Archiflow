#ifndef INTERFACES_H
#define INTERFACES_H

/**
 * @brief Common header for all ArchiFlow interfaces
 * 
 * This header includes all interface definitions used throughout
 * the ArchiFlow application for consistent abstraction patterns.
 */

// Core interfaces
#include "idatabasemanager.h"
#include "imoduleinterface.h"

// Contract feature interfaces
#include "icontractservice.h"
#include "icontractwidget.h"
#include "icontractdialog.h"
#include "icontractexporter.h"
#include "icontractchatbot.h"
#include "icontractimporter.h"

// Future interfaces (to be added as features are developed)
// #include "imaterialservice.h"
// #include "iprojectservice.h"
// #include "iemployeeservice.h"
// #include "iclientservice.h"

#endif // INTERFACES_H
