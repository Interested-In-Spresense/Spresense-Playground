/****************************************************************************
 * isx019_ext.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __DRIVERS_VIDEO_ISX019_EXT_H
#define __DRIVERS_VIDEO_ISX019_EXT_H

/* ISX019 I2C setting */

#define ISX019_I2C_SLVADDR         (0x1a)

/* Command code of ISX019 I2C command format */

#define ISX019_I2C_CMD_READ       (0x01)
#define ISX019_I2C_CMD_WRITE      (0x02)

/* ISX019 I2C parameter */
#define ISX019_I2C_PORT (2)
#define ISX019_I2C_READ_COMMAND_SIZE (9)
#define ISX019_I2C_WRITE_MAX_COMMAND_SIZE (10)

#endif /* __DRIVERS_VIDEO_ISX019_EXT_H */
