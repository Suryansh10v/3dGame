"""
Generates high-resolution publication-quality diagrams and figures for the
Chrono-Dash: Antigravity Arena Project Documentation Report.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image, ImageDraw, ImageFont

os.makedirs('media', exist_ok=True)

# -------------------------------------------------------------------------
# Figure 0: Army Institute of Technology (AIT) Emblem / Logo
# -------------------------------------------------------------------------
def create_ait_emblem():
    fig, ax = plt.subplots(figsize=(4, 4), dpi=300)
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.axis('off')

    # Deep blue theme
    blue_c = '#102C57'
    light_blue = '#1E3E62'

    # Outer triangle
    triangle = patches.Polygon([(-1.2, -0.9), (1.2, -0.9), (0, 1.2)], closed=True, 
                               edgecolor=blue_c, facecolor='none', lw=8, joinstyle='round')
    ax.add_patch(triangle)

    # Inner triangle
    inner_tri = patches.Polygon([(-0.85, -0.7), (0.85, -0.7), (0, 0.85)], closed=True, 
                                edgecolor=blue_c, facecolor='none', lw=4, joinstyle='round')
    ax.add_patch(inner_tri)

    # Central Pillar / Gate emblem
    ax.fill_between([-0.35, 0.35], 0.1, 0.3, color=blue_c)
    ax.fill_between([-0.4, 0.4], 0.25, 0.32, color=blue_c)
    # Vertical pillars
    ax.fill_between([-0.3, -0.15], -0.65, 0.1, color=blue_c)
    ax.fill_between([0.15, 0.3], -0.65, 0.1, color=blue_c)
    ax.fill_between([-0.08, 0.08], -0.65, -0.1, color=blue_c)

    # Banner "Onward to Glory"
    ax.text(0, -1.15, "Onward to Glory", ha='center', va='center', 
            fontsize=13, fontweight='bold', fontfamily='serif', color=blue_c)

    plt.tight_layout()
    plt.savefig('media/ait_logo.png', bbox_inches='tight', transparent=True)
    plt.close()
    print("Created media/ait_logo.png")

# -------------------------------------------------------------------------
# Figure 5.1: Game State & Frame Flow
# -------------------------------------------------------------------------
def create_state_flow():
    fig, ax = plt.subplots(figsize=(9, 4), dpi=300)
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 5)
    ax.axis('off')

    # Draw boxes
    def draw_box(x, y, w, h, title, sub="", color="#102C57", bg="#F0F4F8"):
        rect = patches.FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.1", 
                                      ec=color, fc=bg, lw=2)
        ax.add_patch(rect)
        ax.text(x + w/2, y + h/2 + (0.15 if sub else 0), title, 
                ha='center', va='center', fontsize=10, fontweight='bold', color=color)
        if sub:
            ax.text(x + w/2, y + h/2 - 0.2, sub, 
                    ha='center', va='center', fontsize=8, color="#495057")

    draw_box(0.5, 2.0, 1.8, 1.0, "LAUNCH", "init() + shader compile")
    draw_box(3.2, 2.0, 1.8, 1.0, "SECTOR INTRO", "3.0s banner / key dismissed")
    draw_box(5.8, 2.0, 1.8, 1.0, "PLAYING", "WASD / Mouse / Lasers")

    # Exit branches
    draw_box(8.2, 3.4, 1.6, 0.9, "SECTOR CLEAR", "+100 Kill streak", color="#008080", bg="#E6F4F1")
    draw_box(8.2, 2.0, 1.6, 0.9, "GAME OVER", "Health <= 0 / Void fall", color="#C0392B", bg="#FDEDEC")
    draw_box(8.2, 0.6, 1.6, 0.9, "RESPAWN / RETRY", "Press [R] to retry", color="#D35400", bg="#FEF5E7")

    # Arrows
    def arrow(x1, y1, x2, y2):
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                    arrowprops=dict(arrowstyle="->", color="#102C57", lw=1.8, mutation_scale=14))

    arrow(2.4, 2.5, 3.1, 2.5)
    arrow(5.1, 2.5, 5.7, 2.5)
    arrow(7.7, 2.8, 8.1, 3.7)
    arrow(7.7, 2.5, 8.1, 2.5)
    arrow(7.7, 2.2, 8.1, 1.2)

    # Subtext for pause
    ax.text(5.0, 0.2, "Figure 5.1 Game runtime state machine and transition flow. ESC toggles PAUSE without leaving PLAYING.",
            ha='center', va='center', fontsize=8.5, style='italic', color="#333333")

    plt.tight_layout()
    plt.savefig('media/fig_system_flow.png', bbox_inches='tight')
    plt.close()
    print("Created media/fig_system_flow.png")

# -------------------------------------------------------------------------
# Figure 7.1: Top-down layout of the Chrono-Dash Floating Arena
# -------------------------------------------------------------------------
def create_arena_topdown():
    fig, ax = plt.subplots(figsize=(7, 7), dpi=300)
    ax.set_xlim(-28, 28)
    ax.set_ylim(-28, 28)
    ax.set_facecolor('#05070D')

    # Floor grid background
    for x in range(-25, 26, 5):
        ax.axvline(x, color='#0F172A', lw=0.6, ls='--')
        ax.axhline(x, color='#0F172A', lw=0.6, ls='--')

    # Central Hub Platform (26x26)
    hub = patches.Rectangle((-13, -13), 26, 26, ec='#00D2FF', fc='#0A101D', lw=2)
    ax.add_patch(hub)
    ax.text(0, -6, "CENTRAL PLATFORM (26x26)", ha='center', color='#88A0C0', fontsize=8, fontweight='bold')

    # North Elevated Platform (16x12 at y=6, z=-20)
    north = patches.Rectangle((-8, 14), 16, 12, ec='#FF6B00', fc='#150D05', lw=2)
    ax.add_patch(north)
    ax.text(0, 20, "NORTH PLATFORM\n(16x12, y=6.0)", ha='center', va='center', color='#FF9F45', fontsize=7.5, fontweight='bold')

    # South Elevated Platform (16x12 at y=7.5, z=20)
    south = patches.Rectangle((-8, -26), 16, 12, ec='#00D2FF', fc='#05151F', lw=2)
    ax.add_patch(south)
    ax.text(0, -20, "SOUTH PLATFORM\n(16x12, y=7.5)", ha='center', va='center', color='#00E5FF', fontsize=7.5, fontweight='bold')

    # West High Platform (12x14 at x=-21)
    west = patches.Rectangle((-27, -7), 12, 14, ec='#A855F7', fc='#160A22', lw=2)
    ax.add_patch(west)
    ax.text(-21, 0, "WEST PLATFORM\n(12x14, y=11.5)", ha='center', va='center', color='#C084FC', fontsize=7, fontweight='bold')

    # East High Platform (12x14 at x=21)
    east = patches.Rectangle((15, -7), 12, 14, ec='#A855F7', fc='#160A22', lw=2)
    ax.add_patch(east)
    ax.text(21, 0, "EAST PLATFORM\n(12x14, y=11.5)", ha='center', va='center', color='#C084FC', fontsize=7, fontweight='bold')

    # Corner Stepping Pads (7x7)
    for cx, cy in [(-17.5, 10.5), (10.5, 10.5), (-17.5, -17.5), (10.5, -17.5)]:
        pad = patches.Rectangle((cx, cy), 7, 7, ec='#38BDF8', fc='#081E2E', lw=1.2)
        ax.add_patch(pad)

    # Central Antigravity Core Circle & Distortion field (R = 16.5m)
    field = patches.Circle((0, 0), 16.5, ec='#A855F7', fc='#A855F7', alpha=0.15, ls='--', lw=2)
    ax.add_patch(field)
    core = patches.Circle((0, 0), 3.4, ec='#00E5FF', fc='#9333EA', lw=2.5)
    ax.add_patch(core)
    ax.text(0, 0, "ANTIGRAV\nCORE\n(R=3.4m)", ha='center', va='center', color='#FFFFFF', fontsize=7.5, fontweight='bold')
    ax.text(0, 11, "ZERO-G FIELD RADIUS (R = 16.5m)", ha='center', color='#D8B4FE', fontsize=7.5, style='italic')

    # Player Spawn Dot
    ax.plot(0, 6, 'o', color='#22C55E', markersize=10, markeredgecolor='#FFFFFF', markeredgewidth=1.5)
    ax.text(0, 4.2, "SPAWN (0, 2.5, 6)", ha='center', color='#4ADE80', fontsize=8, fontweight='bold')

    # Enemy Patrol Points
    for ex, ey, label in [(-18, 18, "DRONE 1"), (18, 18, "DRONE 2"), (-18, -18, "DRONE 3"), (18, -18, "DRONE 4")]:
        ax.plot(ex, ey, 'D', color='#EF4444', markersize=8, markeredgecolor='#FFAAAA')
        ax.text(ex, ey + 2.2, label, ha='center', color='#FCA5A5', fontsize=6.5)

    ax.set_title("Figure 7.1 Top-down map of Chrono-Dash Floating Arena\n(Cyan: Lower/Ground, Orange: North, Purple: High Platforms, Purple Dashed: Zero-G Field, Red: Drone Spawns)",
                 fontsize=8.5, color='#CBD5E1', pad=12)

    ax.set_xlabel("X Axis (world units)", color='#94A3B8', fontsize=8)
    ax.set_ylabel("Z Axis (world units)", color='#94A3B8', fontsize=8)
    ax.tick_params(colors='#64748B', labelsize=7)

    plt.tight_layout()
    plt.savefig('media/fig_arena_topdown.png', bbox_inches='tight')
    plt.close()
    print("Created media/fig_arena_topdown.png")

# -------------------------------------------------------------------------
# Figure 9.2: Coordinate Pipeline from Object Space to Window
# -------------------------------------------------------------------------
def create_coord_pipeline():
    fig, ax = plt.subplots(figsize=(10, 2.6), dpi=300)
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 3)
    ax.axis('off')

    stages = [
        ("Vertex v", "object space", 0.5),
        ("Model M", "world = M · v", 2.6),
        ("View V", "eye = V · world", 4.7),
        ("Projection P", "clip = P · eye", 6.8),
        ("Perspective /w", "NDC [-1, 1]³", 8.9),
        ("Viewport", "screen pixels", 10.7)
    ]

    for title, sub, x in stages:
        w = 1.6
        rect = patches.FancyBboxPatch((x - w/2, 0.8), w, 1.4, boxstyle="round,pad=0.08", 
                                      ec='#102C57', fc='#F1F5F9', lw=1.8)
        ax.add_patch(rect)
        ax.text(x, 1.7, title, ha='center', va='center', fontsize=9.5, fontweight='bold', color='#102C57')
        ax.text(x, 1.2, sub, ha='center', va='center', fontsize=8, color='#475569')

    for i in range(len(stages) - 1):
        x1 = stages[i][2] + 0.82
        x2 = stages[i+1][2] - 0.82
        ax.annotate('', xy=(x2, 1.5), xytext=(x1, 1.5),
                    arrowprops=dict(arrowstyle="->", color='#0096C7', lw=2, mutation_scale=12))

    ax.text(6.0, 0.15, "Figure 9.2 Complete 3D transformation and projection coordinate pipeline implemented in Chrono-Dash.",
            ha='center', fontsize=8.5, style='italic', color='#333333')

    plt.tight_layout()
    plt.savefig('media/fig_coord_pipeline.png', bbox_inches='tight')
    plt.close()
    print("Created media/fig_coord_pipeline.png")

# -------------------------------------------------------------------------
# Figure 10.1: Blinn-Phong Vectors & Attenuation Curve
# -------------------------------------------------------------------------
def create_lighting_attenuation():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(9, 4), dpi=300)

    # Left: Vectors
    ax1.set_xlim(-1.2, 1.2)
    ax1.set_ylim(-0.2, 1.4)
    ax1.axis('off')

    # Surface line
    ax1.plot([-1.1, 1.1], [0, 0], color='#334155', lw=3)
    ax1.text(0, -0.12, "Surface Plane", ha='center', color='#475569', fontsize=8)

    # Origin
    O = np.array([0, 0])
    N = np.array([0, 1.0])
    L = np.array([-0.7, 0.75])
    V = np.array([0.7, 0.75])
    H = (L + V) / np.linalg.norm(L + V) * 1.05

    def draw_vec(v, label, color):
        ax1.annotate('', xy=(v[0], v[1]), xytext=(0, 0),
                     arrowprops=dict(arrowstyle="->", color=color, lw=2.2, mutation_scale=12))
        ax1.text(v[0] * 1.12, v[1] * 1.08, label, color=color, fontsize=9.5, fontweight='bold', ha='center')

    draw_vec(N, "N (normal)", "#0284C7")
    draw_vec(L, "L (light dir)", "#EA580C")
    draw_vec(V, "V (view dir)", "#0D9488")
    draw_vec(H, "H (halfway)", "#9333EA")

    ax1.text(0, 1.35, "Halfway vector: H = (L + V) / ||L + V||\nSpecular: (N · H)^shininess", 
             ha='center', fontsize=8, color='#1E293B', bbox=dict(boxstyle="square,pad=0.4", fc="#F8FAFC", ec="#CBD5E1"))

    # Right: Attenuation Curve
    d = np.linspace(0, 50, 200)
    # Chrono-Dash core attenuation: 1 / (1 + 0.04*d + 0.006*d^2)
    att = 1.0 / (1.0 + 0.04 * d + 0.006 * d**2)

    ax2.plot(d, att, color='#7C3AED', lw=2.4, label="1 / (1 + 0.04d + 0.006d²)")
    ax2.set_xlabel("Distance to Antigravity Core (world units)", fontsize=8)
    ax2.set_ylabel("Light Attenuation Factor", fontsize=8)
    ax2.set_title("Core Point-Light Attenuation Curve", fontsize=9.5, fontweight='bold')
    ax2.grid(True, linestyle="--", alpha=0.5)
    ax2.legend(fontsize=8, loc='upper right')
    ax2.set_ylim(0, 1.05)

    plt.suptitle("Figure 10.1 Blinn-Phong lighting geometry vectors (left) and dynamic core point-light attenuation curve (right).",
                 fontsize=8.5, style='italic', y=0.03)
    plt.tight_layout()
    plt.savefig('media/fig_lighting_attenuation.png', bbox_inches='tight')
    plt.close()
    print("Created media/fig_lighting_attenuation.png")

# -------------------------------------------------------------------------
# Figure 11.1: Recoil and Walk Bobbing Waveforms
# -------------------------------------------------------------------------
def create_recoil_bob():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(9, 3.6), dpi=300)

    # Left: Recoil recovery
    t_recoil = np.linspace(0, 0.4, 200)
    # Recoil kicks to 1.0 and decays at rate 8.0: max(0, 1.0 - t * 8.0)
    recoil = np.maximum(0.0, 1.0 - t_recoil * 8.0)
    ax1.plot(t_recoil * 1000, recoil * 0.08 * 100, color='#DC2626', lw=2.4)
    ax1.set_xlabel("Time after firing (milliseconds)", fontsize=8)
    ax1.set_ylabel("Gun Z-kick displacement (cm)", fontsize=8)
    ax1.set_title("Weapon Kickback Recoil Recovery (125ms)", fontsize=9, fontweight='bold')
    ax1.grid(True, linestyle="--", alpha=0.5)
    ax1.fill_between(t_recoil * 1000, recoil * 0.08 * 100, color='#FCA5A5', alpha=0.3)

    # Right: Viewmodel walk bobbing
    t_walk = np.linspace(0, 2 * np.pi, 200)
    bob_x = np.sin(t_walk * 0.8) * 0.8
    bob_y = np.abs(np.cos(t_walk * 0.8)) * 0.7

    ax2.plot(np.degrees(t_walk), bob_x, color='#0284C7', lw=2, label="Lateral sway (X)")
    ax2.plot(np.degrees(t_walk), bob_y, color='#F59E0B', lw=2, label="Vertical bob (Y)")
    ax2.set_xlabel("Walk cycle phase (degrees)", fontsize=8)
    ax2.set_ylabel("Viewmodel camera offset (cm)", fontsize=8)
    ax2.set_title("First-Person Weapon Bobbing Harmonics", fontsize=9, fontweight='bold')
    ax2.grid(True, linestyle="--", alpha=0.5)
    ax2.legend(fontsize=8, loc='upper right')

    plt.suptitle("Figure 11.1 Gun kickback recoil decay curve (left) and viewmodel walking bobbing harmonics (right).",
                 fontsize=8.5, style='italic', y=0.03)
    plt.tight_layout()
    plt.savefig('media/fig_recoil_bob.png', bbox_inches='tight')
    plt.close()
    print("Created media/fig_recoil_bob.png")

# -------------------------------------------------------------------------
# Figure 12.1: Fog Curve
# -------------------------------------------------------------------------
def create_fog_curve():
    fig, ax = plt.subplots(figsize=(6, 3.2), dpi=300)
    dist = np.linspace(0, 140, 200)
    # clamp((dist - 22.0) / 95.0, 0.0, 1.0)
    fog = np.clip((dist - 22.0) / 95.0, 0.0, 1.0)

    ax.plot(dist, fog, color='#0284C7', lw=2.4)
    ax.axvline(22.0, color='#94A3B8', ls='--', lw=1.2)
    ax.axvline(117.0, color='#94A3B8', ls='--', lw=1.2)
    ax.text(23, 0.3, "Fog Start (d = 22m)", color='#475569', fontsize=8, rotation=90)
    ax.text(118, 0.3, "Full Fog (d = 117m)", color='#475569', fontsize=8, rotation=90)

    ax.set_xlabel("Eye-Space Fragment Distance (meters)", fontsize=8)
    ax.set_ylabel("Atmospheric Fog Factor (0 to 1)", fontsize=8)
    ax.set_title("Figure 12.1 Atmospheric distance fog blending factor: clamp((d - 22.0) / 95.0, 0, 1)",
                 fontsize=8.5, style='italic')
    ax.grid(True, linestyle="--", alpha=0.5)
    ax.set_ylim(-0.05, 1.05)

    plt.tight_layout()
    plt.savefig('media/fig_fog_curve.png', bbox_inches='tight')
    plt.close()
    print("Created media/fig_fog_curve.png")

# -------------------------------------------------------------------------
# Figure 14.1: Score and Streak Multiplier Curve
# -------------------------------------------------------------------------
def create_score_curve():
    fig, ax1 = plt.subplots(figsize=(7, 3.5), dpi=300)

    t = np.linspace(0, 12, 300)
    # Kills at t=1, t=3, t=5, t=7, t=9, t=11
    # Streak multiplier step function
    kills = np.array([0, 1, 2, 3, 5, 8, 12])
    kill_times = [0, 1.2, 2.8, 4.5, 6.2, 8.0, 9.8]

    # Calculate streak over time with 4.5s countdown
    mult = []
    score = []
    cur_score = 0
    cur_mult = 1
    cur_streak = 0
    last_hit = -99

    for cur_t in t:
        # Check if hit at this time
        for idx, kt in enumerate(kill_times[1:], 1):
            if abs(cur_t - kt) < 0.02:
                cur_streak += 1
                last_hit = cur_t
                if cur_streak >= 9: cur_mult = 5
                elif cur_streak >= 5: cur_mult = 3
                elif cur_streak >= 2: cur_mult = 2
                else: cur_mult = 1
                cur_score += 100 * cur_mult
                break
        if cur_t - last_hit > 4.5:
            cur_streak = 0
            cur_mult = 1
        mult.append(cur_mult)
        score.append(cur_score)

    ax1.plot(t, score, color='#0284C7', lw=2.4, label="Cumulative Score")
    ax1.set_xlabel("Combat Engagement Time (seconds)", fontsize=8)
    ax1.set_ylabel("Total Score (points)", color='#0284C7', fontsize=8)
    ax1.tick_params(axis='y', labelcolor='#0284C7')
    ax1.grid(True, linestyle="--", alpha=0.5)

    ax2 = ax1.twinx()
    ax2.step(t, mult, color='#F97316', lw=2, where='post', label="Streak Multiplier (1x-5x)")
    ax2.set_ylabel("Active Streak Multiplier", color='#F97316', fontsize=8)
    ax2.tick_params(axis='y', labelcolor='#F97316')
    ax2.set_ylim(0.5, 5.5)

    plt.title("Figure 14.1 Score accumulation with consecutive kill streak multipliers (1x -> 2x -> 3x -> 5x)",
              fontsize=8.5, style='italic', pad=10)
    plt.tight_layout()
    plt.savefig('media/fig_score_curve.png', bbox_inches='tight')
    plt.close()
    print("Created media/fig_score_curve.png")

if __name__ == "__main__":
    create_ait_emblem()
    create_state_flow()
    create_arena_topdown()
    create_coord_pipeline()
    create_lighting_attenuation()
    create_recoil_bob()
    create_fog_curve()
    create_score_curve()
    print("All report figures generated successfully in media/!")
