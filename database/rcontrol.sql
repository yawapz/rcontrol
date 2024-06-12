CREATE DATABASE rcontrol
    WITH
    OWNER = postgres
    ENCODING = 'UTF8'
    LOCALE_PROVIDER = 'libc'
    CONNECTION LIMIT = -1
    IS_TEMPLATE = False;
    
CREATE TABLE public.users
(
    id uuid NOT NULL,
    login text NOT NULL,
    password text NOT NULL,
    last_visit timestamp without time zone NOT NULL,
    PRIMARY KEY (id)
);

ALTER TABLE IF EXISTS public.users
    OWNER to postgres;
    
CREATE TABLE public.workers
(
    id uuid NOT NULL,
    user_id uuid NOT NULL,
    gpu_list_id uuid[],
    worker_name text,
    electricity_cost float,
    kernel_version text,
    nvidia_version text,
    amd_version text,
    motherboard_info text,
    cpu_info text,
    disk_model text,
    disk_size text,
    ram_total text,
    mac text,
    local_ip text,
    ext_ip text,
    last_online timestamp without time zone,
    PRIMARY KEY (id)
);

ALTER TABLE IF EXISTS public.workers
    OWNER to postgres;
    
CREATE TABLE public.gpus
(
    id uuid NOT NULL,
    worker_id uuid NOT NULL,
    set_fan_speed smallint,
    set_core smallint,
    set_mem integer,
    set_pl smallint,
    name text,
    bus_id text,
    vendor text,
    total_memory text,
    vbios_version text,
    min_pl text,
    default_pl text,
    max_pl text,
    gpu_sys_id smallint,
    PRIMARY KEY (id)
);

ALTER TABLE IF EXISTS public.gpus
    OWNER to postgres;
