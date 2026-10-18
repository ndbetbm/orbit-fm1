#!/usr/bin/env python3
"""Build the ORBIT static site; omit --package for an honest browser-only preview."""
import argparse, hashlib, html, json, re, shutil, sys, zipfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'web'))
from make_site import main as upstream_site, strip_module

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--package', type=Path)
    ap.add_argument('--out', type=Path, required=True)
    a = ap.parse_args()
    out = a.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    if a.package:
        upstream_site(a.package, '1.0', out)
    else:
        if list((out / 'firmware').glob('*.fwsc')):
            raise SystemExit('Use a fresh output directory for a preview without firmware.')
        for folder in ['webapp/installer', 'webapp/editor', 'firmware']:
            (out / folder).mkdir(parents=True, exist_ok=True)
        lib = '\n'.join(strip_module((ROOT/'web'/f).read_text()) for f in ['fm1pkg.js','fm1ota.js','fm1backup.js'])
        page = (ROOT/'web/index_pkg.html').read_text().replace('/*LIB*/', lib).replace('/*META*/', json.dumps(dict(version='1.0', product=None, pkg=None)))
        (out/'webapp/installer/index.html').write_text(page)
        shutil.copy(ROOT/'web/editor.html', out/'webapp/editor/index.html')
        for f in ['fukiai.ttf','FUKIAI-LICENSE.txt','fm1backup.js']:
            if (ROOT/'web'/f).exists(): shutil.copy(ROOT/'web'/f, out/'webapp/editor'/f)
        shutil.copytree(ROOT/'LICENSES', out/'firmware/LICENSES', dirs_exist_ok=True)
        for f in ['LICENSE','LICENSING.md']: shutil.copy(ROOT/f, out/'firmware'/f)
        (out/'firmware/LICENSES/index.html').write_text('<meta charset="utf-8"><h1>Licenses</h1>' + ''.join(f'<p><a href="{p.name}">{p.name}</a></p>' for p in sorted((out/'firmware/LICENSES').glob('*.txt'))))
    installer = out/'webapp/installer/index.html'
    page = installer.read_text().replace('<title>Felucca</title>','<title>ORBIT 1.0 — 更新と復帰</title>').replace('<h1>Felucca</h1>', '<h1>ORBIT 1.0</h1><p>実機での書き込み・起動・純正復帰は未検証です。</p><p><a href="../../">トップへ</a> · <a href="../../downloads/ORBIT-1.0-source.zip">この版のソース</a></p>')
    page = page.replace('<details style="margin-top:24px">','<details id="stock" style="margin-top:24px">')
    page = page.replace('    const r = await fetch(meta.pkg);', '    if (!meta.pkg) { say("ORBIT 1.0：実機用パッケージはビルド待ちです。"); return; }\n    const r = await fetch(meta.pkg);')
    digest = hashlib.sha256(a.package.read_bytes()).hexdigest() if a.package else None
    page = page.replace('    image = logicalImage(pkg);', f'    if (await sha256hex(pkg) !== {json.dumps(digest)}) throw new Error("SHA-256 mismatch");\n    image = logicalImage(pkg);')
    page = page.replace('say(`Felucca ${meta.version}`)', 'say(`ORBIT ${meta.version} / 実機未検証`)')
    page = page.replace('</script>', 'if (location.hash === "#stock") document.getElementById("stock").open = true;\n</script>')
    page = page.replace('Felucca 1.0 は実地テスト版です。不具合は GitHub で知らせてください。', 'ORBIT 1.0 は実機未検証です。')
    page = page.replace('Felucca 1.0 is a field-testing release; please report problems on GitHub.', 'ORBIT 1.0 has not been tested on hardware.')
    page = page.replace('エディタ（インストール後、パソコンから音色やシーケンスを編集）', 'バックアップ管理（上流エディター）')
    page = page.replace('Editor (after installing: edit sounds and sequences from the computer)', 'Backup management (upstream editor)')
    installer.write_text(page)
    for f in ['index.html','style.css','site.js','guide.html','manual.html','manual.css']:
        shutil.copy(ROOT/'web/orbit-site'/f, out/f)
    target = out/'try'
    target.mkdir(exist_ok=True)
    for f in ['index.html','worklet.js','felucca.wasm']:
        shutil.copy(ROOT/'build/emu'/f, target/f)
    shutil.copytree(ROOT/'build/emu/fonts', target/'fonts', dirs_exist_ok=True)
    (out/'release.json').write_text(json.dumps(dict(version='1.0', firmwareReady=bool(a.package), hardwareTested=False, sha256=digest), indent=2))
    (out/'.nojekyll').touch()
    (out/'downloads').mkdir(exist_ok=True)
    with zipfile.ZipFile(out/'downloads/ORBIT-1.0-source.zip','w',zipfile.ZIP_DEFLATED) as z:
        for p in sorted(ROOT.rglob('*')):
            rel = p.relative_to(ROOT)
            if not p.is_file() or any(x in {'.git','build','__pycache__','.gitmodules'} for x in rel.parts) or out in p.parents: continue
            z.write(p, Path('ORBIT-1.0')/rel)
    print('ORBIT site:', out, '| firmwareReady:', bool(a.package))
if __name__ == '__main__': main()
