/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <apr_general.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "bundle_activator.h"
#include "bundle_context.h"
#include "simple_shape.h"
#include "paint_frame.h"
#include "service_tracker.h"

struct paintFrameActivatorData {
	SERVICE_REGISTRATION reg;
	apr_pool_t *pool;
	SERVICE_TRACKER tracker;
	BUNDLE_CONTEXT context;
	PAINT_FRAME paint_frame;
};
void *addingServ(void * handle, SERVICE_REFERENCE reference);
void addedServ(void * handle, SERVICE_REFERENCE reference, void * service);
void modifiedServ(void * handle, SERVICE_REFERENCE reference, void * service);
void removedServ(void * handle, SERVICE_REFERENCE reference, void * service);

typedef struct paintFrameActivatorData *GREETING_ACTIVATOR;

celix_status_t bundleActivator_create(BUNDLE_CONTEXT context, void **userData) {
	apr_pool_t *pool;
	GREETING_ACTIVATOR activator;
	SERVICE_TRACKER_CUSTOMIZER cust;
	printf("Paint_frame create\n");
	celix_status_t status = bundleContext_getMemoryPool(context, &pool);
	if (status == CELIX_SUCCESS) {
		*userData = apr_palloc(pool, sizeof(struct paintFrameActivatorData));
		activator = *userData;
		activator->reg = NULL;
		activator->pool = pool;
		activator->context = context;
		activator->paint_frame = paintFrame_create(context, pool);
        cust = (SERVICE_TRACKER_CUSTOMIZER) apr_palloc(pool, sizeof(*cust));
        cust->handle = activator;
        cust->addedService = addedServ;
        cust->addingService = addingServ;
        cust->modifiedService = modifiedServ;
        cust->removedService = removedServ;
		tracker_create(context, SIMPLE_SHAPE_SERVICE_NAME, cust, &activator->tracker);
		tracker_open(activator->tracker);
	}
	return status;
}

celix_status_t bundleActivator_start(void * userData, BUNDLE_CONTEXT ctx) {
	struct paintFrameActivatorData * act = (struct paintFrameActivatorData *) userData;
	celix_status_t status = CELIX_SUCCESS;
    status = bundleContext_registerService(ctx, PAINT_FRAME_SERVICE_NAME,
    		act->paint_frame, NULL, &act->reg);
	return status;
}

celix_status_t bundleActivator_stop(void * userData, BUNDLE_CONTEXT context) {
	struct paintFrameActivatorData * act = (struct paintFrameActivatorData *) userData;
	tracker_close(act->tracker);
	return CELIX_SUCCESS;
}

celix_status_t bundleActivator_destroy(void * userData, BUNDLE_CONTEXT context) {
	struct paintFrameActivatorData * act = (struct paintFrameActivatorData *) userData;
	return CELIX_SUCCESS;
}

void *addingServ(void * handle, SERVICE_REFERENCE ref) {
	struct paintFrameActivatorData * data = (struct paintFrameActivatorData *) handle;
    void *svc = NULL;
    bundleContext_getService(data->context, ref, &svc);
    return svc;
}

void addedServ(void * handle, SERVICE_REFERENCE ref, void * service) {
	struct paintFrameActivatorData * data = (struct paintFrameActivatorData *) handle;
	char * serviceName = properties_get(ref->registration->properties, "name");
	data->paint_frame->paintFrame_addShape(data->context, serviceName, NULL, service);
 }

void modifiedServ(void * handle, SERVICE_REFERENCE ref, void * service) {
	struct paintFrameActivatorData * data = (struct paintFrameActivatorData *) handle;
	char * serviceName = properties_get(ref->registration->properties, "name");
}

void removedServ(void * handle, SERVICE_REFERENCE ref, void * service) {
	struct paintFrameActivatorData * data = (struct paintFrameActivatorData *) handle;
	char * serviceName = properties_get(ref->registration->properties, "name");
	data->paint_frame->paintFrame_removeShape(serviceName);
}
