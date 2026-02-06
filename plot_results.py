#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt

def load_graph1():
    rows = []
    with open('graph1_data.csv') as f:
        r = csv.DictReader(f)
        for row in r:
            rows.append({
                'stream_size': int(row['stream_size']),
                'step': int(row['step']),
                't': float(row['t']),
                'processed_size': int(row['processed_size']),
                'Ft0': int(row['Ft0']),
                'Nt': int(row['Nt'])
            })
    return rows

def load_graph2():
    rows = []
    with open('graph2_data.csv') as f:
        r = csv.DictReader(f)
        for row in r:
            rows.append({
                'stream_size': int(row['stream_size']),
                'step': int(row['step']),
                't': float(row['t']),
                'processed_size': int(row['processed_size']),
                'E_Ft0': float(row['E_Ft0']),
                'E_Nt': float(row['E_Nt']),
                'sigma_Nt': float(row['sigma_Nt'])
            })
    return rows

def plot_graph1(data):
    sizes = sorted(set(r['stream_size'] for r in data))
    fig, axes = plt.subplots(2, 2, figsize=(10, 8))
    axes = axes.flatten()
    for idx, sz in enumerate(sizes):
        ax = axes[idx]
        subset = [r for r in data if r['stream_size'] == sz]
        steps = [r['step'] for r in subset]
        proc = [r['processed_size'] for r in subset]
        ft0 = [r['Ft0'] for r in subset]
        nt = [r['Nt'] for r in subset]
        ax.plot(proc, ft0, 'b-o', label='Ft0 (точное)', markersize=6)
        ax.plot(proc, nt, 'r--s', label='Nt (оценка)', markersize=6)
        ax.set_xlabel('Размер обработанной части')
        ax.set_ylabel('Число уникальных элементов')
        ax.set_title(f'Размер потока = {sz}')
        ax.legend()
        ax.grid(True, alpha=0.3)
    plt.suptitle('График 1: Сравнение Ft0 и Nt')
    plt.tight_layout()
    plt.savefig('graph1_ft0_vs_nt.png', dpi=150)
    plt.close()

def plot_graph2(data):
    sizes = sorted(set(r['stream_size'] for r in data))
    fig, axes = plt.subplots(2, 2, figsize=(10, 8))
    axes = axes.flatten()
    for idx, sz in enumerate(sizes):
        ax = axes[idx]
        subset = [r for r in data if r['stream_size'] == sz]
        proc = [r['processed_size'] for r in subset]
        e_nt = [r['E_Nt'] for r in subset]
        sigma = [r['sigma_Nt'] for r in subset]
        upper = [e + s for e, s in zip(e_nt, sigma)]
        lower = [e - s for e, s in zip(e_nt, sigma)]
        ax.fill_between(proc, lower, upper, alpha=0.3)
        ax.plot(proc, e_nt, 'b-o', label='E(Nt)', markersize=6)
        ax.set_xlabel('Размер обработанной части')
        ax.set_ylabel('E(Nt)')
        ax.set_title(f'Размер потока = {sz}')
        ax.legend()
        ax.grid(True, alpha=0.3)
    plt.suptitle('График 2: E(Nt) с областью неопределённости ±σ')
    plt.tight_layout()
    plt.savefig('graph2_E_Nt_sigma.png', dpi=150)
    plt.close()

def main():
    g1 = load_graph1()
    g2 = load_graph2()
    plot_graph1(g1)
    plot_graph2(g2)
    print('Сохранены graph1_ft0_vs_nt.png и graph2_E_Nt_sigma.png')

if __name__ == '__main__':
    main()
