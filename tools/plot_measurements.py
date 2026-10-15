#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
# Copyright (c) 2026 Neurora and contributors.
from pathlib import Path
import csv
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
rows=list(csv.DictReader((Path(__file__).resolve().parents[1]/'docs/data/aester-response.csv').open()))
plt.rcParams.update({'font.family':'DejaVu Sans','text.color':'#dce9e2','axes.labelcolor':'#b1c8bf','xtick.color':'#b1c8bf','ytick.color':'#b1c8bf'})
fig,ax=plt.subplots(figsize=(10,4.5),facecolor='#101a1c',layout='constrained');ax.set_facecolor('#142225')
colors=['#d9e8dc','#bce6c9','#80c1d4','#dfa773','#a69cce']
for case,color in zip(dict.fromkeys(r['case'] for r in rows),colors):
 points=[r for r in rows if r['case']==case];ax.semilogx([float(r['frequency_hz']) for r in points],[float(r['gain_db']) for r in points],'o-',ms=3,color=color,label=case)
ax.set(xlabel='Frequency (Hz)',ylabel='Measured gain (dB)',ylim=(-50,10),title='Aester EQ / 48 kHz sine response');ax.grid(alpha=.15);ax.legend(fontsize=8,labelcolor='#dce9e2',facecolor='#142225',edgecolor='#30463f',loc='lower left')
fig.savefig(Path(__file__).resolve().parents[1]/'assets/aester-measurements.png',dpi=160)
