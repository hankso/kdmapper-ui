#!/usr/bin/env python3
# coding=utf-8
#
# File: kdmapper_ui.py
# Author: Hankso
# Webpage: https://github.com/hankso
# Time: 2025/7/18 11:03:50

# nuitka-project: --onefile
# nuitka-project: --output-dir=build
# nuitka-project: --output-filename=kdmapper-ui
# nuitka-project: --enable-plugin=tk-inter
# nuitka-project: --windows-uac-admin
# nuitka-project: --windows-console-mode=disable
# nuitka-project: --include-data-files=kdmapper.exe=./
# nuitka-project: --include-data-files=HelloWorld.sys=./

import sys
import glob
import ctypes
import subprocess
import os.path as op
import tkinter as tk
from tkinter import ttk, filedialog, scrolledtext
from datetime import datetime
from threading import Thread


def fromroot(*a, search=True):
    if search:
        tpl = op.join(op.dirname(__file__), '**', a[-1])
        lst = glob.glob(tpl, recursive=True)
        if lst:
            return lst[0]
    return op.abspath(op.join(op.dirname(__file__), *a))


class KdmapperUI:
    KDMAPPER = fromroot('kdmapper.exe')
    HELLOWORLD = fromroot('HelloWorld.sys')

    def state(self, elem, state):
        if not isinstance(state, str):
            state = '!disabled' if state else 'disabled'
        if hasattr(elem, 'state'):
            elem.state([state])
        else:
            elem['state'] = 'normal' if state[0] == '!' else state

    def enable(self, elem): self.state(elem, '!disabled')
    def disable(self, elem): self.state(elem, 'disabled')
    def valid_file(self, path): return op.isfile(path) and op.getsize(path)

    def __init__(self, root):
        self.root = root
        self.root.title('KDMapper UI')
        self.root.minsize(500, 500)

        self.stat = tk.StringVar()
        self.path = tk.StringVar(value=self.HELLOWORLD)
        self.opts = [tk.BooleanVar() for i in range(3)]

        fmain = ttk.Frame(self.root, padding=15)
        fmain.pack(fill=tk.BOTH, expand=True)
        fmain.rowconfigure(2, weight=1)
        fmain.columnconfigure(0, weight=1)

        fpath = ttk.LabelFrame(fmain, text='选择驱动', padding=10)
        fpath.grid(row=0, sticky='we', pady=5)
        fpath.columnconfigure(0, weight=1)

        epath = ttk.Entry(fpath, textvariable=self.path)
        bpath = ttk.Button(fpath, text='浏览', command=self.select_driver)
        epath.grid(row=0, column=0, sticky='we', padx=5, pady=5)
        bpath.grid(row=0, column=1, sticky='e', padx=5, pady=5)

        fopts = ttk.LabelFrame(fmain, text='加载参数', padding=10)
        fopts.grid(row=1, sticky='we', pady=5)
        fopts.columnconfigure(0, weight=1)
        fopts.columnconfigure(1, weight=1)
        for i, (text, var) in enumerate(zip([
            '传递占用的内存的指针作为参数 (--PassAllocationPtr)',
            '加载完成后自动释放占用的内存 (--free)',
            '在独立的内存页中进行驱动映射 (--indPages)',
        ], self.opts)):
            b = ttk.Checkbutton(fopts, text=text, variable=var)
            b.grid(row=i, column=0, sticky='we', padx=5, pady=5)

        flogs = ttk.LabelFrame(fmain, text='打印信息', padding=15)
        flogs.grid(row=2, sticky='wens', pady=5)
        self.tlog = scrolledtext.ScrolledText(flogs, height=1, wrap=tk.WORD)
        self.rbtn = ttk.Button(flogs, text='加载驱动', command=self.execute)
        self.cbtn = ttk.Button(flogs, text='清空信息', command=self.log_clr)
        self.tlog.pack(fill=tk.BOTH, expand=True, padx=5, pady=(5, 10))
        self.rbtn.pack(side=tk.LEFT)
        self.cbtn.pack(side=tk.RIGHT)
        self.disable(self.tlog)

        lstat = ttk.Label(fmain, textvariable=self.stat)
        lstat.grid(row=3, sticky='we', pady=(5, 0))

        self.path.trace_add('write', lambda *a: self.state(
            self.rbtn, self.valid_file(self.path.get())
        ))
        self.stat.trace_add('write', lambda *a: lstat.config(
            foreground=['green', 'red']['错误' in self.stat.get().lower()]
        ))
        if not self.valid_file(self.HELLOWORLD):
            self.path.set('')
        if not self.valid_file(self.KDMAPPER):
            self.disable(fmain)
            self.stat.set('错误: kdmapper 可能被杀软删掉，尝试关闭软件重开!')
        else:
            self.stat.set('就绪')

    def select_driver(self):
        path = filedialog.askopenfilename(
            filetypes=[('驱动文件', '*.sys'), ('所有文件', '*.*')])
        if path:
            self.path.set(path)

    def log_msg(self, msg):
        tstr = datetime.now().strftime("%H:%M:%S.%f")
        self.enable(self.tlog)
        self.tlog.insert(tk.END, f'[{tstr[:-3]}] {msg.strip()}\n')
        self.tlog.see(tk.END)
        self.disable(self.tlog)

    def log_clr(self):
        self.enable(self.tlog)
        self.tlog.delete(1.0, tk.END)
        self.disable(self.tlog)

    def execute(self):
        self.disable(self.rbtn)
        cmd = [self.KDMAPPER]
        if self.opts[0].get():
            cmd.append('--PassAllocationPtr')
        if self.opts[1].get():
            cmd.append('--free')
        elif self.opts[2].get():
            cmd.append('--indPages')
        cmd.append(self.path.get())
        admin = ctypes.windll.shell32.IsUserAnAdmin()
        if not admin:
            cmd = ['runas', '/user:Administrator'] + [f'"{i}"' for i in cmd]
        self.log_msg('运行命令: ' + ' '.join(cmd))
        Thread(target=self.run, args=(cmd, admin), daemon=True).start()

    def run(self, cmd, admin, **k):
        try:
            k.setdefault('bufsize', 1)
            k.setdefault('text', True)
            k.setdefault('shell', not admin)
            k.setdefault('stdout', subprocess.PIPE)
            k.setdefault('stderr', subprocess.STDOUT)
            k.setdefault('universal_newlines', True)
            self.proc = subprocess.Popen(cmd, **k)
            for line in iter(self.proc.stdout.readline, ''):
                self.root.after(0, self.log_msg, line)
            rc = self.proc.wait(timeout=10)
            if rc:
                self.root.after(0, self.log_msg, f'错误: exit code={rc}')
                self.root.after(0, self.stat.set, '错误')
            else:
                self.root.after(0, self.log_msg, '加载完成')
                self.root.after(0, self.stat.set, '就绪')
        except Exception as e:
            self.root.after(0, self.log_msg, f'错误: {str(e)}')
            self.root.after(0, self.stat.set, '错误')
        finally:
            self.root.after(0, self.enable, self.rbtn)
            if getattr(self, 'proc', None):
                self.proc.kill()


def main():
    app = KdmapperUI(tk.Tk())
    try:
        app.root.mainloop()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == '__main__':
    sys.exit(main())
