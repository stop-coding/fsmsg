#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import sys
import os

def main():
    if not _check_python_version():
        print("python need version 3 or more")
        os._exit(1)
    if len(sys.argv) < 3:
        print("input invalid, need proto file")
        print("exp:", sys.argv[0], "<proto file dir> ", "<code root dir> ")
        os._exit(1)
    
    proto_path = sys.argv[1]
    if not os.path.isdir(proto_path):
        print("proto path:", proto_path, " invalid.")
        print("exp:", sys.argv[0], "<proto file dir> ", "<code root dir> ")
        os._exit(1)
    out_path = sys.argv[2]
    if not os.path.isdir(out_path):
        print("proto file:", out_path, " invalid.")
        print("exp:", sys.argv[0], "<proto file dir> ", "<code root dir> ")
        os._exit(1)

    out_path = os.path.realpath(out_path)
    head_export_ctx=""
    ops_export_ctx=""
    head_include = ""
    for proto_file in os.listdir(proto_path):

        proto_file = os.path.realpath(os.path.join(proto_path, proto_file))

        (file_path, tempfilename) = os.path.split(proto_file)
        (proto_name, extension) = os.path.splitext(tempfilename)
        if extension != ".proto":
            continue

        ret = os.system("protoc-c --c_out=" + file_path + " --proto_path=" + file_path + " " + tempfilename)
        if ret != 0:
            print("error: protoc-c exce fail.")
            continue
        if not os.path.isfile(os.path.join(file_path, proto_name + ".pb-c.h")):
            print("error: can't create file ", os.path.join(file_path, proto_name + ".pb-c.h"))
            continue
        print("message box:", proto_name)
        msg_set = _get_per_message_name_id(proto_file, proto_name)
        if msg_set is None:
            print("_get_per_message_name_id fail")
            os._exit(1)

        msg_header_str = _get_msg_header(msg_set, proto_name)
        if msg_header_str is None:
            print("_add_header fail.", proto_name)
            continue
        head_export_ctx = head_export_ctx + msg_header_str
        head_include = head_include + "#include \"" + proto_name +".pb-c.h\"\n"
        ret = os.system('mv ' + os.path.join(file_path, proto_name + ".pb-c.h") + " " + os.path.join(out_path, "inc"))
        if ret != 0:
            print("error: mv file fail.")
            continue
        msg_ops_str = _get_msg_ops(msg_set, proto_name)
        if msg_ops_str is None:
            print("_add_header fail.", proto_name)
            continue
        ops_export_ctx = ops_export_ctx + msg_ops_str
        os.system('mv ' + os.path.join(file_path, proto_name + ".pb-c.c") + " " + os.path.join(out_path, "src"))

    if not _insert_header(file_path, head_include, head_export_ctx, os.path.join(out_path, "inc")):
        print("_insert_header fail.")
        os._exit(1) 
    
    if not _insert_ops(file_path, ops_export_ctx, os.path.join(out_path, "src")):
        print("_insert_ops fail.")
        os._exit(1)
    return 0

def _get_msg_header(msg_set, proto_name):
    export_string = "EXPORT_MESSAGE_BOX(\n"+proto_name+",\n"
    index = 1
    for key in msg_set:
        if index != 1:
            export_string = export_string + ",\n"
        index = index + 1
        msg_id_str =  "MSG_ID_"+ msg_set[key].upper()
        if key != index and key > 1:
            export_string=export_string + msg_id_str + " = " + str(key)
        else:
            export_string=export_string + msg_id_str

    export_string = export_string + ");\n\n"
    
    return export_string

def _insert_header(file_path, head_include, export_string, out_path):
    header_templete = os.path.join(file_path, "msg_h_templete.tem")
    if not os.path.isfile(header_templete):
        print("header templete:", header_templete, " not exits.")
        return False

    out_file = os.path.join(out_path, "fsmsg.h")
    with open(out_file, 'w') as o:
        with open(header_templete, 'r') as f:
            head_ctx = f.read()
            head_ctx =  '#ifndef _FSMSG_H_ \n#define _FSMSG_H_\n\n'+ head_include + head_ctx + export_string + '#ifdef __cplusplus\n}\n#endif\n#endif\n'
            o.write(head_ctx)
    return True

def covet2hump(in_str):
    ret = "(" + in_str +", "
    hump = filter(None, in_str.lower().split('_'))
    res = ''
    for i in hump:
        res = res + i[0].upper() + i[1:]
    ret = ret + res + ")"
    return ret

def _get_msg_ops(msg_set, proto_name):
    define_pb = "\n"
    index = 0
    for key in msg_set:
        index = index + 1
        define_pb = define_pb + "DEFINE_PROTO_MESSAGE_OPS"+ covet2hump(msg_set[key]) +"\n"

    define_pb = define_pb + "\nINIT_MESSAGE_BOX(\n" + proto_name +",\n"
    index = 0
    for key in msg_set:
        index = index + 1
        msg_id_str =  "MSG_ID_"+ msg_set[key].upper()
        define_pb = define_pb + "REGISTER_MESSAGE("+ msg_id_str + ", " +msg_set[key] +")\n"
    define_pb = define_pb + ")\n\n"
    return define_pb

def _insert_ops(file_path, define_pb, out_path):
    ops_templete = os.path.join(file_path, "msg_c_templete.tem")
    if not os.path.isfile(ops_templete):
        print("ops templete:", ops_templete, " not exits.")
        return False
    
    out_file = os.path.join(out_path, "fsmsg.c")
    with open(out_file, 'w') as o:
        with open(ops_templete, 'r') as f:
            ops_ctx = f.read()
            ops_ctx = ops_ctx + define_pb
            o.write(ops_ctx)
    return True

def _check_python_version():
    major_ver = sys.version_info[0]
    if major_ver < 3:
        return False
    return True
def _get_per_message_name_id(file_name, proto_name):
    if not os.path.isfile(file_name):
        print("file:",file_name, " not exist.")
        return None
    msg_table = {}
    index = 1
    with open(file_name) as fr:
        for line in fr.readlines():
            line=line.lstrip()
            if not line.startswith("message"):
                continue
            message_name = "none"
            for line_name in line.split():
                if not line_name.startswith(proto_name):
                    continue
                message_name = line_name
            if message_name == "none":
                continue
            message_id = index
            for line_name in line.split():
                if not line_name.startswith("//##"):
                    continue
                num_id = line_name.replace("//##",'')
                num_id = num_id.strip()
                if not num_id.isnumeric():
                    print("num_id:[", num_id, "] must be numeric")
                    break
                message_id = int(num_id)
                index = message_id
            index =index+1
            if msg_table.get(message_id) is not None:
                print("msg id:[",message_id, "] is repeat on message[", message_name, "].")
                return None
            msg_table[message_id] = message_name
            print("name:", msg_table[message_id], "id:", message_id)
    return msg_table

if __name__ == "__main__":
    # execute only if run as a script
    main()