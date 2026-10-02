#pragma once

struct CallbackTable;
struct ClassNode;
struct Element;

struct PlaneRideNode;
void FUN_0043d9f0(struct PlaneRideNode *node);
void FUN_0043d990(struct PlaneRideNode *node);
void FUN_0043e410(struct Element *elem);

void PlaneRide_GetInterfaces(struct ClassNode *name, struct CallbackTable *iface);
